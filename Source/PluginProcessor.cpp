/*
  ==============================================================================
    Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
    Project: Zherfai AI - Autonomous Multi-Node Mix & Master Engine
    Component: Channel Strip Core Execution
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ZherfaiAudioEngineProcessor::ZherfaiAudioEngineProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout()),
      forwardFFT (fftOrder),
      window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    fifo.fill (0.0f);
    fftData.fill (0.0f);
    initializeSharedMemory();
}

ZherfaiAudioEngineProcessor::~ZherfaiAudioEngineProcessor()
{
    cleanupSharedMemory();
}

juce::AudioProcessorValueTreeState::ParameterLayout ZherfaiAudioEngineProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterBool> ("auto_gain", "Auto Gain Stage (-18dB)", true));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("auto_demud", "Acoustic De-Mudding", true));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("auto_deess", "Dynamic De-Esser", true));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("stem_override", "Stem Mode Override", 
        juce::StringArray { "AUTO_DETECT", "SUB_LOW", "VOCAL_LEAD", "PERCUSSION", "HARMONIC_PADS" }, 0));
    return { params.begin(), params.end() };
}

void ZherfaiAudioEngineProcessor::initializeSharedMemory()
{
#if JUCE_WINDOWS
    hMapFile = CreateFileMappingA (INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(ZherfaiMeshMatrix), ZHERFAI_SHM_NAME);
    if (hMapFile != NULL)
    {
        pSharedMatrix = (ZherfaiMeshMatrix*) MapViewOfFile (hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ZherfaiMeshMatrix));
        if (pSharedMatrix != nullptr)
        {
            for (int i = 0; i < ZHERFAI_MAX_CHANNELS; ++i)
            {
                bool expected = false;
                if (pSharedMatrix->channels[i].isActive.compare_exchange_strong (expected, true))
                {
                    assignedNodeIndex = i;
                    pSharedMatrix->channels[i].channelId.store (i + 1);
                    pSharedMatrix->totalActiveNodes.fetch_add (1);
                    break;
                }
            }
        }
    }
#endif
}

void ZherfaiAudioEngineProcessor::cleanupSharedMemory()
{
#if JUCE_WINDOWS
    if (pSharedMatrix != nullptr && assignedNodeIndex >= 0)
    {
        pSharedMatrix->channels[assignedNodeIndex].isActive.store (false);
        pSharedMatrix->totalActiveNodes.fetch_sub (1);
        UnmapViewOfFile (pSharedMatrix);
        pSharedMatrix = nullptr;
    }
    if (hMapFile != NULL)
    {
        CloseHandle (hMapFile);
        hMapFile = NULL;
    }
#endif
}

void ZherfaiAudioEngineProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());

    // 1. Sub-Cut Filter: 30 Hz steep highpass
    *subCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 32.0f, 0.707f);
    subCutFilter.prepare (spec);

    // 2. De-Mud Filter: Variable dynamic notch targeting 320 Hz
    *deMudFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 320.0f, 2.5f, 0.707f);
    deMudFilter.prepare (spec);

    // 3. De-Esser Bandpass Monitor: Centered at 6.8 kHz
    *deEsserBandpass.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, 6800.0f, 1.8f);
    deEsserBandpass.prepare (spec);

    smoothedGainDb.reset (sampleRate, 0.05); // 50ms smoothing window to eliminate clicking
    smoothedGainDb.setCurrentAndTargetValue (0.0f);
}

void ZherfaiAudioEngineProcessor::releaseResources() {}

bool ZherfaiAudioEngineProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void ZherfaiAudioEngineProcessor::computeSpectralAcoustics()
{
    window.multiplyWithWindowingTable (fftData.data(), fftSize);
    forwardFFT.performFrequencyOnlySpectrum (fftData.data());

    float binWidth = static_cast<float> (currentSampleRate) / static_cast<float> (fftSize);
    float subEnergy = 0.0f;
    float midEnergy = 0.0f;
    float highPresenceEnergy = 0.0f;
    float totalEnergy = 1e-6f;
    float weightedSum = 0.0f;

    for (int i = 0; i < fftSize / 2; ++i)
    {
        float freq = i * binWidth;
        float power = fftData[i] * fftData[i];
        totalEnergy += power;
        weightedSum += freq * power;

        if (freq >= 20.0f && freq <= 130.0f)
            subEnergy += power;
        else if (freq > 130.0f && freq <= 1500.0f)
            midEnergy += power;
        else if (freq >= 2000.0f && freq <= 5500.0f)
            highPresenceEnergy += power;
    }

    float centroid = weightedSum / totalEnergy;
    
    // Automatic Role Classification
    auto overrideChoice = apvts.getRawParameterValue ("stem_override")->load();
    if (overrideChoice == 0) // AUTO_DETECT
    {
        if (subEnergy / totalEnergy > 0.45f)
            currentStem = STEM_SUB_LOW;
        else if (highPresenceEnergy / totalEnergy > 0.28f && centroid > 1800.0f)
            currentStem = STEM_VOCAL_LEAD;
        else if (midEnergy / totalEnergy > 0.50f)
            currentStem = STEM_HARMONIC_PADS;
        else
            currentStem = STEM_PERCUSSION;
    }
    else
    {
        currentStem = static_cast<StemType> (static_cast<int> (overrideChoice));
    }

    if (pSharedMatrix != nullptr && assignedNodeIndex >= 0)
    {
        pSharedMatrix->channels[assignedNodeIndex].detectedStem.store (currentStem);
        pSharedMatrix->channels[assignedNodeIndex].spectralCentroidHz.store (centroid);
    }
}

void ZherfaiAudioEngineProcessor::processDeEsser (juce::AudioBuffer<float>& buffer)
{
    juce::AudioBuffer<float> sidechainBuffer;
    sidechainBuffer.makeCopyOf (buffer);

    juce::dsp::AudioBlock<float> block (sidechainBuffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    deEsserBandpass.process (context);

    float sibilanceEnergy = sidechainBuffer.getRMSLevel (0, 0, sidechainBuffer.getNumSamples());
    if (sidechainBuffer.getNumChannels() > 1)
        sibilanceEnergy = std::max (sibilanceEnergy, sidechainBuffer.getRMSLevel (1, 0, sidechainBuffer.getNumSamples()));

    float threshold = 0.08f; // Dynamic sibilance limit
    if (sibilanceEnergy > threshold)
    {
        float overDb = juce::Decibels::gainToDecibels (sibilanceEnergy / threshold);
        float attenuation = std::clamp (1.0f - (overDb * 0.08f), 0.55f, 1.0f);
        deEsserGainReduction = deEsserGainReduction * 0.7f + attenuation * 0.3f; // Ultra-fast attack
    }
    else
    {
        deEsserGainReduction = deEsserGainReduction * 0.95f + 1.0f * 0.05f; // Fast release
    }

    if (currentStem == STEM_VOCAL_LEAD && apvts.getRawParameterValue ("auto_deess")->load())
    {
        buffer.applyGain (deEsserGainReduction);
    }
}

void ZherfaiAudioEngineProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    // 1. Feature Extraction Stream
    const float* channelData = buffer.getReadPointer (0);
    int zeroCrossings = 0;
    for (int i = 0; i < numSamples; ++i)
    {
        if (fifoIndex < fftSize)
            fifo[fifoIndex++] = channelData[i];

        if (fifoIndex == fftSize)
        {
            std::copy (fifo.begin(), fifo.end(), fftData.begin());
            std::fill (fftData.begin() + fftSize, fftData.end(), 0.0f);
            computeSpectralAcoustics();
            fifoIndex = 0;
        }

        if (i > 0 && ((channelData[i] >= 0.0f && channelData[i-1] < 0.0f) || (channelData[i] < 0.0f && channelData[i-1] >= 0.0f)))
            zeroCrossings++;
    }

    // 2. Continuous RMS & Crest Factor
    float rms = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        rms = std::max (rms, buffer.getRMSLevel (ch, 0, numSamples));

    float rmsDb = juce::Decibels::gainToDecibels (rms + 1e-5f);
    float peakDb = juce::Decibels::gainToDecibels (buffer.getMagnitude (0, numSamples) + 1e-5f);
    float crestFactor = peakDb - rmsDb;

    // 3. Auto Gain Staging (-18 dBFS RMS Anchor)
    if (apvts.getRawParameterValue ("auto_gain")->load() && rmsDb > -50.0f)
    {
        float targetGainDiff = -18.0f - rmsDb;
        targetGainDiff = std::clamp (targetGainDiff, -12.0f, 12.0f);
        smoothedGainDb.setTargetValue (targetGainDiff);
    }
    else
    {
        smoothedGainDb.setTargetValue (0.0f);
    }

    float currentGain = juce::Decibels::decibelsToGain (smoothedGainDb.getNextValue());
    buffer.applyGain (currentGain);

    // 4. Local Cleansing: Sub-Cut & Acoustic De-Mudding
    juce::dsp::AudioBlock<float> audioBlock (buffer);
    juce::dsp::ProcessContextReplacing<float> context (audioBlock);

    subCutFilter.process (context);

    if (apvts.getRawParameterValue ("auto_demud")->load())
        deMudFilter.process (context);

    // 5. Dynamic De-Esser Execution
    processDeEsser (buffer);

    // 6. External Control Injection from Master Brain via Mesh IPC
    if (pSharedMatrix != nullptr && assignedNodeIndex >= 0)
    {
        pSharedMatrix->channels[assignedNodeIndex].currentRmsDb.store (rmsDb);
        pSharedMatrix->channels[assignedNodeIndex].crestFactorDb.store (crestFactor);
        pSharedMatrix->channels[assignedNodeIndex].zeroCrossingRate.store (static_cast<float> (zeroCrossings) / numSamples);

        // Check Phase Alignment Signal
        if (pSharedMatrix->channels[assignedNodeIndex].phaseInversionRequested.load())
        {
            buffer.applyGain (-1.0f);
        }

        // Apply Dynamic Sidechain Notch Ducking
        float duckDb = pSharedMatrix->channels[assignedNodeIndex].sidechainDuckGainDb.load();
        if (duckDb < -0.05f)
        {
            buffer.applyGain (juce::Decibels::decibelsToGain (duckDb));
        }
    }
}

void ZherfaiAudioEngineProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ZherfaiAudioEngineProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorEditor* ZherfaiAudioEngineProcessor::createEditor()
{
    return new ZherfaiAudioEngineEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZherfaiAudioEngineProcessor();
}
