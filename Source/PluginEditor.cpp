/*
  ==============================================================================
  Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
  Project: Zherfai AI - Autonomous Mixing & Mastering VST3 Suite
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ZherfaiAudioEngineEditor::ZherfaiAudioEngineEditor (ZherfaiAudioEngineProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (500, 320);

    titleLabel.setText ("ZHERFAI AI MASTERING", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::cyan);
    addAndMakeVisible (titleLabel);

    statusLabel.setText ("Autonomous Engine: ACTIVE\n[ -18dB RMS Auto-Gain | Clarity Filter | -1.0 dBTP Limiter ]", juce::dontSendNotification);
    statusLabel.setFont (juce::FontOptions (14.0f));
    statusLabel.setJustificationType (juce::Justification::centred);
    statusLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (statusLabel);

    attributionLabel.setText ("Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity", juce::dontSendNotification);
    attributionLabel.setFont (juce::FontOptions (11.0f));
    attributionLabel.setJustificationType (juce::Justification::centred);
    attributionLabel.setColour (juce::Label::textColourId, juce::Colours::darkgrey);
    addAndMakeVisible (attributionLabel);
}

ZherfaiAudioEngineEditor::~ZherfaiAudioEngineEditor()
{
}

void ZherfaiAudioEngineEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12141a)); // Dark modern slate background

    g.setColour (juce::Colours::cyan.withAlpha (0.4f));
    g.drawRoundedRectangle (15.0f, 15.0f, getWidth() - 30.0f, getHeight() - 30.0f, 10.0f, 1.5f);
}

void ZherfaiAudioEngineEditor::resized()
{
    titleLabel.setBounds (20, 30, getWidth() - 40, 35);
    statusLabel.setBounds (20, 100, getWidth() - 40, 70);
    attributionLabel.setBounds (20, getHeight() - 35, getWidth() - 40, 20);
}
