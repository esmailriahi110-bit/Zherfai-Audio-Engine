/*
  ==============================================================================
    Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
    Project: Zherfai AI - Autonomous Multi-Node Mix & Master Engine
    Component: Channel Strip Studio Display
  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class ZherfaiAudioEngineEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    ZherfaiAudioEngineEditor (ZherfaiAudioEngineProcessor&);
    ~ZherfaiAudioEngineEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    ZherfaiAudioEngineProcessor& audioProcessor;
    juce::Label titleLabel;
    juce::Label stemLabel;
    juce::Label gainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZherfaiAudioEngineEditor)
};
