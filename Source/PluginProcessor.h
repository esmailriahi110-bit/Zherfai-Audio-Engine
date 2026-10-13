/*
  ==============================================================================
  Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
  Project: Zherfai AI - Autonomous Mixing & Mastering VST3 Suite
  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class ZherfaiAudioEngineProcessor : public juce::AudioProcessor
{
public:
    ZherfaiAudioEngineProcessor();
    ~ZherfaiAudioEngineProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Processors
    juce::dsp::ProcessorChain<
        juce::dsp::Gain<float>,
        juce::dsp::IIR::Filter<float>, // Mud cut (400-800Hz)
        juce::dsp::IIR::Filter<float>, // Presence boost (2-5kHz)
        juce::dsp::Limiter<float>
    > dspChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZherfaiAudioEngineProcessor)
};
