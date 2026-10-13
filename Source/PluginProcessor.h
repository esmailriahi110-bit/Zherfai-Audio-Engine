/*
  ==============================================================================
    Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
    Project: Zherfai AI - Autonomous Multi-Node Mix & Master Engine
    Component: Zherfai Channel Strip Processor
  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "ZherfaiSharedMemory.h"

#if JUCE_WINDOWS
#include <windows.h>
#endif

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
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Zherfai Channel Strip"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    StemType getDetectedStemType() const { return currentStem; }
    float getTargetGainDb() const { return smoothedGainDb.getCurrentValue(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // IPC Link
    void initializeSharedMemory();
    void cleanupSharedMemory();
    void updateTelemetryToMaster();

    #if JUCE_WINDOWS
    HANDLE hMapFile = NULL;
    ZherfaiMeshMatrix* pSharedMatrix = nullptr;
    #else
    ZherfaiMeshMatrix* pSharedMatrix = nullptr;
    #endif

    int assignedNodeIndex = -1;
    StemType currentStem = STEM_UNKNOWN;

    // Feature Extraction Ring & Buffers
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    int fifoIndex = 0;
    std::atomic<bool> nextFFTBlockReady { false };

    // DSP Processors
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> subCutFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> deMudFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> deEsserBandpass;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedGainDb;
    float deEsserGainReduction = 1.0f;
    double currentSampleRate = 44100.0;

    void computeSpectralAcoustics();
    void processDeEsser (juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZherfaiAudioEngineProcessor)
};
