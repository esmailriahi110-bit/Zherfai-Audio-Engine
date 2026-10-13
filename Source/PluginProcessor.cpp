/*
  ==============================================================================
  Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
  Project: Zherfai AI - Autonomous Mixing & Mastering VST3 Suite
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ZherfaiAudioEngineProcessor::ZherfaiAudioEngineProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

ZherfaiAudioEngineProcessor::~ZherfaiAudioEngineProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ZherfaiAudioEngineProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "auto_gain", "Auto Gain Stage (-18dB RMS)", 0.0f, 1.0f, 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "vocal_clarity", "Vocal Clarity Engine", 0.0f, 100.0f, 50.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "limiter_ceiling", "True-Peak Ceiling (dBTP)", -2.0f, -0.1f, -1.0f));

    return { params.begin(), params.end() };
}

const juce::String ZherfaiAudioEngineProcessor::getName() const { return JucePlugin_Name; }
bool ZherfaiAudioEngineProcessor::acceptsMidi() const { return false; }
bool ZherfaiAudioEngineProcessor::producesMidi() const { return false; }
bool ZherfaiAudioEngineProcessor::isMidiEffect() const { return false; }
double ZherfaiAudioEngineProcessor::getTailLengthSeconds() const { return 0.0; }
int ZherfaiAudioEngineProcessor::getNumPrograms() { return 1; }
int ZherfaiAudioEngineProcessor::getCurrentProgram() { return 0; }
void ZherfaiAudioEngineProcessor::setCurrentProgram (int) {}
const juce::String ZherfaiAudioEngineProcessor::getProgramName (int) { return {}; }
void ZherfaiAudioEngineProcessor::changeProgramName (int, const juce::String&) {}

void ZherfaiAudioEngineProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());

    dspChain.prepare (spec);

    // Mud reduction filter (~500Hz Notch/Dip)
    *dspChain.get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        sampleRate, 500.0f, 1.0f, 0.75f);

    // Presence engine filter (~3.5kHz Boost)
    *dspChain.get<2>().coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        sampleRate, 3500.0f, 1.0f, 1.25f);

    // EBU R128 True-Peak Limiter (-1.0 dBTP default)
    dspChain.get<3>().setThreshold (-1.0f);
    dspChain.get<3>().setRelease (100.0f);
}

void ZherfaiAudioEngineProcessor::releaseResources()
{
}

bool ZherfaiAudioEngineProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void ZherfaiAudioEngineProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Auto Gain Staging (-18 dB RMS safety target)
    float rms = buffer.getRMSLevel (0, 0, buffer.getNumSamples());
    if (rms > 0.0001f)
    {
        float targetGain = 0.126f / rms; // 0.126 corresponds to roughly -18 dB RMS
        targetGain = juce::jlimit (0.25f, 4.0f, targetGain); // Safe Guardrail
        buffer.applyGain (targetGain);
    }

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    dspChain.process (context);
}

bool ZherfaiAudioEngineProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* ZherfaiAudioEngineProcessor::createEditor()
{
    return new ZherfaiAudioEngineEditor (*this);
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
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZherfaiAudioEngineProcessor();
}
