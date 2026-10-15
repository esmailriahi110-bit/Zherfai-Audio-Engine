#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include "ZherfaiSharedMemory.h"

#if JUCE_WINDOWS
#include <windows.h>
#endif

class ZherfaiAudioEngineAudioProcessor : public juce::AudioProcessor
{
public:
    ZherfaiAudioEngineAudioProcessor();
    ~ZherfaiAudioEngineAudioProcessor() override;

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
    void setCurrentProgram (int /*index*/) override {}
    const juce::String getProgramName (int /*index*/) override { return {}; }
    void changeProgramName (int /*index*/, const juce::String& /*newName*/) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // متدهای کمکی برای UI
    float getSmoothedGain() const { return smoothedGain.getTargetValue(); }
    ZherfaiStemType getCurrentStem() const { return detectedStem.load(); }
    bool isConnectedToMesh() const { return pSharedMatrix != nullptr; }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Components
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> subCutFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> deMudFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> deEssFilter;
    
    juce::LinearSmoothedValue<float> smoothedGain;

    // FFT & Analysis
    enum { fftOrder = 10, fftSize = 1 << fftOrder };
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    std::array<float, fftSize> fftData;
    int fifoIndex = 0;

    // Shared Memory IPC
    #if JUCE_WINDOWS
    HANDLE hMapFile = nullptr;
    #endif
    ZherfaiMeshMatrix* pSharedMatrix = nullptr;
    int assignedNodeIndex = -1;
    std::atomic<ZherfaiStemType> detectedStem{STEM_UNKNOWN};

    void initSharedMemory();
    void cleanupSharedMemory();
    void updateTelemetryAndMesh(const juce::AudioBuffer<float>& buffer);
    void processDeEsserAndDynamics(juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZherfaiAudioEngineAudioProcessor)
};
