#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class ZherfaiAudioEngineEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ZherfaiAudioEngineEditor (ZherfaiAudioEngineAudioProcessor&);
    ~ZherfaiAudioEngineEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    ZherfaiAudioEngineAudioProcessor& audioProcessor;

    // استایل دکمه‌ها و عناصر کنترلی
    juce::ToggleButton autoGainButton { "AUTO GAIN (-18 dBFS)" };
    juce::ToggleButton deMudButton   { "DE-MUD (320 Hz)" };
    juce::ToggleButton deEssButton   { "SMART DE-ESSER" };
    juce::ComboBox     stemSelector;

    // اتچمنت‌های APVTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deMudAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deEssAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> stemAttachment;

    float currentMeterLevel = 0.0f;
    float currentGainReduction = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZherfaiAudioEngineEditor)
};
