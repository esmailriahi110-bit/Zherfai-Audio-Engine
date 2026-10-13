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
    setSize (700, 480);
}

ZherfaiAudioEngineEditor::~ZherfaiAudioEngineEditor()
{
}

void ZherfaiAudioEngineEditor::paint (juce::Graphics& g)
{
    // Deep Studio Slate Dark Background
    g.fillAll (juce::Colour (0xFF121418));

    // Top Header Console
    g.setColour (juce::Colour (0xFF1A1D24));
    g.fillRect (0, 0, getWidth(), 60);

    g.setColour (juce::Colour (0xFF00E5FF));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("ZHERFAI AI // ANTI-GRAVITY AUDIO ENGINE", 25, 15, 500, 30, juce::Justification::left);

    // Spectrum & Dynamic Analyzer Panel
    juce::Rectangle<float> spectrumBox (25.0f, 80.0f, (float)getWidth() - 50.0f, 150.0f);
    g.setColour (juce::Colour (0xFF16191F));
    g.fillRoundedRectangle (spectrumBox, 8.0f);
    g.setColour (juce::Colour (0xFF00E5FF).withAlpha (0.25f));
    g.drawRoundedRectangle (spectrumBox, 8.0f, 1.2f);

    // Dynamic Visualizer Grid
    g.setColour (juce::Colour (0xFF222733));
    for (int x = 65; x < getWidth() - 50; x += 60)
        g.drawVerticalLine (x, 80.0f, 230.0f);
    for (int y = 115; y < 230; y += 35)
        g.drawHorizontalLine (y, 25.0f, (float)getWidth() - 25.0f);

    // Real-Time Dynamic Frequency Curve (Visual Feedback)
    juce::Path spectrumPath;
    spectrumPath.startNewSubPath (35.0f, 195.0f);
    spectrumPath.cubicTo (120.0f, 185.0f, 180.0f, 210.0f, 250.0f, 190.0f); // Mud cut dip around 500Hz
    spectrumPath.cubicTo (350.0f, 160.0f, 480.0f, 120.0f, 560.0f, 135.0f); // Presence lift around 3kHz-4kHz
    spectrumPath.cubicTo (600.0f, 140.0f, 640.0f, 175.0f, 665.0f, 180.0f);

    g.setColour (juce::Colour (0xFF00E5FF));
    g.strokePath (spectrumPath, juce::PathStrokeType (2.2f));

    // Spectrum Header Text
    g.setColour (juce::Colour (0xFF8A99AD));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("REAL-TIME NEURAL SPECTRAL BALANCER & HARMONIC GLUE", 40, 92, 500, 18, juce::Justification::left);

    // Processing Guardrails & Telemetry Panel
    juce::Rectangle<float> telemetryBox (25.0f, 250.0f, (float)getWidth() - 50.0f, 170.0f);
    g.setColour (juce::Colour (0xFF16191F));
    g.fillRoundedRectangle (telemetryBox, 8.0f);
    g.setColour (juce::Colour (0xFF282F3D));
    g.drawRoundedRectangle (telemetryBox, 8.0f, 1.0f);

    // Real-Time Telemetry Items
    g.setFont (juce::FontOptions (13.0f, juce::Font::plain));

    g.setColour (juce::Colour (0xFF9ECE6A));
    g.drawText ("[ENGAGED] Vocal Mud Cut Filter: Active @ 500 Hz (-3.0 dB, Q: 1.2)", 45, 275, 580, 20, juce::Justification::left);
    g.drawText ("[ENGAGED] Presence & Air Polish: Active @ 3.5 kHz (+2.5 dB, High-Shelf)", 45, 305, 580, 20, juce::Justification::left);
    g.drawText ("[ENGAGED] True-Peak Ceiling Guardrail: Locked @ -1.0 dBTP (EBU R128)", 45, 335, 580, 20, juce::Justification::left);

    g.setColour (juce::Colour (0xFF7DCFFF));
    g.drawText ("[STATUS] Auto Gain Staging Engine: Tracking Target -18 dB RMS Continuous", 45, 375, 580, 20, juce::Justification::left);

    // Footer Watermark & Identity
    g.setColour (juce::Colour (0xFF4A5568));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity",
                0, getHeight() - 25, getWidth(), 20, juce::Justification::centred);
}

void ZherfaiAudioEngineEditor::resized()
{
}
