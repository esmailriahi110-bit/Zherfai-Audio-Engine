/*
  ==============================================================================
  Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
  Project: Zherfai AI - Autonomous Mixing & Mastering VST3 Suite
  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class ZherfaiAudioEngineEditor : public juce::AudioProcessorEditor
{
public:
    ZherfaiAudioEngineEditor (ZherfaiAudioEngineProcessor&);
    ~ZherfaiAudioEngineEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ZherfaiAudioEngineProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label attributionLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZherfaiAudioEngineEditor)
};
