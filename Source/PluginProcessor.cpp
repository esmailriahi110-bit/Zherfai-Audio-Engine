#include "PluginProcessor.h"
#include "PluginEditor.h"

ZherfaiAudioEngineAudioProcessor::ZherfaiAudioEngineAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout()),
      forwardFFT (fftOrder),
      window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    initSharedMemory();
}

ZherfaiAudioEngineAudioProcessor::~ZherfaiAudioEngineAudioProcessor()
{
    cleanupSharedMemory();
}

juce::AudioProcessorValueTreeState::ParameterLayout ZherfaiAudioEngineAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterBool> ("auto_gain", "Auto Gain Staging", true));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("auto_demud", "Acoustic De-Mudding", true));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("auto_deess", "Dynamic De-Esser", true));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("stem_override", "Stem Role Override", 
        juce::StringArray{"Auto", "Sub/Kick", "Vocal Lead", "Percussion", "Harmonic/Pads"}, 0));
    return { params.begin(), params.end() };
}

void ZherfaiAudioEngineAudioProcessor::initSharedMemory()
{
#if JUCE_WINDOWS
    hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(ZherfaiMeshMatrix), ZHERFAI_SHM_NAME);
    if (hMapFile != NULL) {
        pSharedMatrix = static_pointer_cast<ZherfaiMeshMatrix*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ZherfaiMeshMatrix)));
        if (pSharedMatrix != nullptr) {
            // تخصیص یک اسلات خالی برای این کانال
            for (int i = 0; i < ZHERFAI_MAX_CHANNELS; ++i) {
                int expected = 0;
                if (std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic<int>*>(&pSharedMatrix->channels[i].isActive), &expected, 1)) {
                    assignedNodeIndex = i;
                    pSharedMatrix->channels[i].channelId = i;
                    pSharedMatrix->totalActiveNodes++;
                    break;
                }
            }
        }
    }
#endif
}

void ZherfaiAudioEngineAudioProcessor::cleanupSharedMemory()
{
#if JUCE_WINDOWS
    if (pSharedMatrix != nullptr && assignedNodeIndex >= 0) {
        pSharedMatrix->channels[assignedNodeIndex].isActive = 0;
        pSharedMatrix->totalActiveNodes = std::max(0, pSharedMatrix->totalActiveNodes.load() - 1);
        UnmapViewOfFile(pSharedMatrix);
    }
    if (hMapFile != NULL) {
        CloseHandle(hMapFile);
    }
#endif
}

void ZherfaiAudioEngineAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 2048;
    spec.numChannels = (uint32) getTotalNumOutputChannels();

    subCutFilter.prepare(spec);
    *subCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 30.0f, 0.707f);

    deMudFilter.prepare(spec);
    *deMudFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 320.0f, 1.5f, -3.0f);

    deEssFilter.prepare(spec);
    *deEssFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 6800.0f, 2.0f, -4.0f);

    smoothedGain.reset(sampleRate, 0.05);
    smoothedGain.setCurrentAndTargetValue(1.0f);
}

void ZherfaiAudioEngineAudioProcessor::releaseResources() {}

bool ZherfaiAudioEngineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

void ZherfaiAudioEngineAudioProcessor::updateTelemetryAndMesh(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumSamples() == 0) return;

    const float* channelData = buffer.getReadPointer(0);
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        if (fifoIndex < fftSize) {
            fftData[fifoIndex++] = channelData[i];
        }
    }

    if (fifoIndex >= fftSize) {
        window.multiplyWithWindowingTable(fftData.data(), fftSize);
        forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

        // تحلیل طیف فرکانسی برای تعیین نقش لاین (Stem Classification)
        float subEnergy = 0.0f, midEnergy = 0.0f, highEnergy = 0.0f;
        for (int i = 1; i < fftSize / 2; ++i) {
            float freq = i * (float)getSampleRate() / (float)fftSize;
            if (freq < 150.0f) subEnergy += fftData[i];
            else if (freq >= 150.0f && freq < 3000.0f) midEnergy += fftData[i];
            else if (freq >= 3000.0f) highEnergy += fftData[i];
        }

        int overrideChoice = static_cast<int>(*apvts.getRawParameterValue("stem_override"));
        if (overrideChoice == 0) {
            if (subEnergy > midEnergy && subEnergy > highEnergy) detectedStem.store(STEM_SUB_LOW);
            else if (highEnergy > midEnergy * 1.2f) detectedStem.store(STEM_VOCAL_LEAD);
            else if (subEnergy > highEnergy * 2.0f) detectedStem.store(STEM_PERCUSSION);
            else detectedStem.store(STEM_HARMONIC_PADS);
        } else {
            detectedStem.store(static_cast<ZherfaiStemType>(overrideChoice));
        }

        if (pSharedMatrix != nullptr && assignedNodeIndex >= 0) {
            pSharedMatrix->channels[assignedNodeIndex].stemType = detectedStem.load();
            pSharedMatrix->channels[assignedNodeIndex].currentRmsDb = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
        }

        fifoIndex = 0;
    }
}

void ZherfaiAudioEngineAudioProcessor::processDeEsserAndDynamics(juce::AudioBuffer<float>& buffer)
{
    if (*apvts.getRawParameterValue("auto_deess") && detectedStem.load() == STEM_VOCAL_LEAD) {
        juce::ScopedNoDenormals noDenormals;
        juce::dsp::AudioBlock<float> block(buffer);
        deEssFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }
}

void ZherfaiAudioEngineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateTelemetryAndMesh(buffer);

    // ۱. گین استیجینگ خودکار روی -18 dBFS RMS هدف
    if (*apvts.getRawParameterValue("auto_gain")) {
        float currentRms = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
        float targetRms = 0.125f; // معادل ~18- dBFS
        if (currentRms > 0.001f) {
            float neededGain = targetRms / currentRms;
            neededGain = juce::jlimit(0.25f, 4.0f, neededGain); // محدودسازی برای جلوگیری از پامپینگ شدید
            smoothedGain.setTargetValue(neededGain);
        }
    } else {
        smoothedGain.setTargetValue(1.0f);
    }

    buffer.applyGain(smoothedGain.getNextSample(buffer.getNumSamples()));

    // ۲. فیلترهای جراحی و پاک‌سازی آکوستیک
    juce::dsp::AudioBlock<float> block(buffer);
    
    // ساب‌کات ۳۰ هرتز برای همه‌ی لاین‌ها جهت پاک‌سازی بیس‌های هدررفته
    subCutFilter.process(juce::dsp::ProcessContextReplacing<float>(block));

    if (*apvts.getRawParameterValue("auto_demud")) {
        deMudFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    // ۳. پردازش داینامیک و دی‌اسر اختصاصی وکال
    processDeEsserAndDynamics(buffer);

    // ۴. اعمال دستورات سایدچین دریافتی از شبکه مش
    if (pSharedMatrix != nullptr && assignedNodeIndex >= 0) {
        float duckGain = pSharedMatrix->channels[assignedNodeIndex].sidechainDuckGainDb;
        if (duckGain < 0.0f) {
            buffer.applyGain(juce::Decibels::decibelsToGain(duckGain));
        }
    }
}

void ZherfaiAudioEngineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ZherfaiAudioEngineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}
