/*
  ==============================================================================
    Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
    Project: Zherfai AI - Autonomous Multi-Node Mix & Master Engine
    Component: Channel Strip Studio Display
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ZherfaiAudioEngineEditor::ZherfaiAudioEngineEditor (ZherfaiAudioEngineProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (640, 380);

    titleLabel.setText ("ZHERFAI MESH // NODE CHANNEL STRIP", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF00E5FF));
    addAndMakeVisible (titleLabel);

    stemLabel.setText ("ROLE: ANALYZING...", juce::dontSendNotification);
    stemLabel.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    stemLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF9ECE6A));
    addAndMakeVisible (stemLabel);

    gainLabel.setText ("GAIN OFFSET: 0.0 dB", juce::dontSendNotification);
    gainLabel.setFont (juce::FontOptions (13.0f, juce::Font::plain));
    gainLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF7DCFFF));
    addAndMakeVisible (gainLabel);

    startTimerHz (24); // 24 FPS UI Refresh
}

ZherfaiAudioEngineEditor::~ZherfaiAudioEngineEditor()
{
    stopTimer();
}

void ZherfaiAudioEngineEditor::timerCallback()
{
    StemType stem = audioProcessor.getDetectedStemType();
    juce::String stemStr = "ROLE: ";
    switch (stem)
    {
        case STEM_SUB_LOW:      stemStr += "SUB_LOW [KICK/BASS]"; break;
        case STEM_VOCAL_LEAD:   stemStr += "VOCAL_LEAD [ACTIVE POCKET]"; break;
        case STEM_PERCUSSION:   stemStr += "PERCUSSION [TRANSIENT FOCUS]"; break;
        case STEM_HARMONIC_PADS:stemStr += "HARMONIC_PADS [MID-SUSTAIN]"; break;
        default:                stemStr += "ANALYZING ACOUSTICS..."; break;
    }
    stemLabel.setText (stemStr, juce::dontSendNotification);

    float gainDb = audioProcessor.getTargetGainDb();
    gainLabel.setText ("AUTO GAIN COMP: " + juce::String (gainDb, 1) + " dB (Target -18 dBFS)", juce::dontSendNotification);

    repaint();
}

void ZherfaiAudioEngineEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF0F1115));

    // Console Enclosure
    juce::Rectangle<float> mainBox (20.0f, 60.0f, (float)getWidth() - 40.0f, (float)getHeight() - 100.0f);
    g.setColour (juce::Colour (0xFF161920));
    g.fillRoundedRectangle (mainBox, 6.0f);
    g.setColour (juce::Colour (0xFF262D3D));
    g.drawRoundedRectangle (mainBox, 6.0f, 1.2f);

    // Active Processing Stack
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xFF8A99AD));
    g.drawText ("ACTIVE NODE REAL-TIME PROTOCOLS", 40, 80, 400, 20, juce::Justification::left);

    g.setFont (juce::FontOptions (12.0f, juce::Font::plain));
    g.setColour (juce::Colour (0xFFE0AF68));
    g.drawText ("[*] Sub-Cut 32Hz (18 dB/oct) Steep High-Pass Active", 40, 190, 500, 20, juce::Justification::left);
    g.drawText ("[*] De-Mudding Filter: Dynamic Notch Monitoring @ 320 Hz", 40, 220, 500, 20, juce::Justification::left);
    g.drawText ("[*] Dynamic De-Esser: Fast-Attack Peak Attenuation (6.8 kHz Band)", 40, 250, 500, 20, juce::Justification::left);
    g.drawText ("[*] Mesh IPC Shared Memory: Connected to Master Brain Bus", 40, 280, 500, 20, juce::Justification::left);

    // Signature
    g.setColour (juce::Colour (0xFF4A5568));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity",
                0, getHeight() - 25, getWidth(), 20, juce::Justification::centred);
}

void ZherfaiAudioEngineEditor::resized()
{
    titleLabel.setBounds (20, 15, 450, 25);
    stemLabel.setBounds (40, 115, 500, 25);
    gainLabel.setBounds (40, 145, 500, 25);
}
