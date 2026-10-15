#include "PluginProcessor.h"
#include "PluginEditor.h"

ZherfaiAudioEngineEditor::ZherfaiAudioEngineEditor (ZherfaiAudioEngineAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (720, 460);

    // استایل‌دهی سلکتور استم
    stemSelector.addItem ("Auto Detect (AI)", 1);
    stemSelector.addItem ("Sub / 808 / Kick", 2);
    stemSelector.addItem ("Lead Vocal", 3);
    stemSelector.addItem ("Drums / Percussion", 4);
    stemSelector.addItem ("Harmonics / Instruments", 5);
    stemSelector.setSelectedId (1);
    addAndMakeVisible (stemSelector);

    // دکمه‌های مدرن وضعیت پردازش
    auto configureButton = [this](juce::ToggleButton& btn) {
        btn.setColour(juce::ToggleButton::textColourId, juce::Colour(0xFFD6E2F0));
        btn.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFF00FFCC));
        addAndMakeVisible(btn);
    };

    configureButton(autoGainButton);
    configureButton(deMudButton);
    configureButton(deEssButton);

    // اتصال به متغیرهای DSP
    autoGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "auto_gain", autoGainButton);
    deMudAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "auto_demud", deMudButton);
    deEssAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "auto_deess", deEssButton);
    stemAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "stem_override", stemSelector);

    startTimerHz (30); // رفرش روان ۳۰ فریم بر ثانیه برای میترها
}

ZherfaiAudioEngineEditor::~ZherfaiAudioEngineEditor()
{
    stopTimer();
}

void ZherfaiAudioEngineEditor::timerCallback()
{
    // دریافت میزان خروجی و پردازش از موتور برای انیمیشن
    currentGainReduction = audioProcessor.getSmoothedGain();
    repaint();
}

void ZherfaiAudioEngineEditor::paint (juce::Graphics& g)
{
    // ۱. پس‌زمینه گرادیان عمیق استودیویی
    juce::ColourGradient bgGradient(juce::Colour(0xFF0B0E14), 0, 0,
                                   juce::Colour(0xFF141A24), 0, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // ۲. هدر بالایی (Header Panel)
    g.setColour(juce::Colour(0xFF1B2332));
    g.fillRect(0, 0, getWidth(), 64);

    g.setColour(juce::Colour(0xFF00FFCC));
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.drawText("ZHERFAI AUDIO ENGINE", 24, 12, 350, 24, juce::Justification::left);

    g.setColour(juce::Colour(0xFF6B7D96));
    g.setFont(juce::Font(12.0f, juce::Font::plain));
    g.drawText("AUTONOMOUS MESH MIXER // ARCHITECTURE v2.5", 25, 36, 350, 18, juce::Justification::left);

    // وضعیت اتصال به شبکه مش (Mesh Status Pill)
    bool meshConnected = audioProcessor.isConnectedToMesh();
    juce::Colour statusCol = meshConnected ? juce::Colour(0xFF00FF88) : juce::Colour(0xFFFF5555);
    g.setColour(statusCol.withAlpha(0.2f));
    g.fillRoundedRectangle(getWidth() - 175.0f, 18.0f, 150.0f, 28.0f, 14.0f);
    g.setColour(statusCol);
    g.drawRoundedRectangle(getWidth() - 175.0f, 18.0f, 150.0f, 28.0f, 14.0f, 1.2f);
    g.fillEllipse(getWidth() - 162.0f, 28.0f, 8.0f, 8.0f);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(meshConnected ? "MESH LINKED" : "STANDALONE", getWidth() - 148, 18, 115, 28, juce::Justification::centredLeft);

    // ۳. پنل اصلی مانیتورینگ صوتی
    auto monitorArea = juce::Rectangle<float>(24.0f, 80.0f, getWidth() - 48.0f, 220.0f);
    g.setColour(juce::Colour(0xFF0E121A));
    g.fillRoundedRectangle(monitorArea, 8.0f);
    g.setColour(juce::Colour(0xFF222B3D));
    g.drawRoundedRectangle(monitorArea, 8.0f, 1.5f);

    // تشخیص زنده استم (Stem Badge)
    juce::String stemStr = "ANALYZING...";
    switch (audioProcessor.getCurrentStem()) {
        case STEM_SUB_LOW:       stemStr = "ROLE: SUB / BASS ANCHOR"; break;
        case STEM_VOCAL_LEAD:    stemStr = "ROLE: LEAD VOCAL FOCUS"; break;
        case STEM_PERCUSSION:    stemStr = "ROLE: PERCUSSION PUNCH"; break;
        case STEM_HARMONIC_PADS: stemStr = "ROLE: INSTRUMENT HARMONY"; break;
        default: break;
    }

    g.setColour(juce::Colour(0xFF00D4FF));
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(stemStr, 40, 96, 400, 20, juce::Justification::left);

    // خط نمایش نئونی گین خودکار
    g.setColour(juce::Colour(0xFF6B7D96));
    g.setFont(juce::Font(12.0f, juce::Font::plain));
    g.drawText("AUTO GAIN TARGET: -18 dBFS", 40, 130, 250, 18, juce::Justification::left);

    float meterWidth = (getWidth() - 96.0f);
    float gainCompWidth = juce::jlimit(10.0f, meterWidth, (currentGainReduction / 2.0f) * meterWidth);

    g.setColour(juce::Colour(0xFF1B2230));
    g.fillRoundedRectangle(40.0f, 155.0f, meterWidth, 14.0f, 7.0f);

    juce::ColourGradient meterGrad(juce::Colour(0xFF00FFCC), 40.0f, 155.0f,
                                 juce::Colour(0xFF0088FF), 40.0f + gainCompWidth, 155.0f, false);
    g.setGradientFill(meterGrad);
    g.fillRoundedRectangle(40.0f, 155.0f, gainCompWidth, 14.0f, 7.0f);

    // راهنمای ماژول‌های فعال
    g.setColour(juce::Colour(0xFF90A4AE));
    g.setFont(juce::Font(11.0f, juce::Font::plain));
    g.drawText("[DSP ACTIVE] Phase Alignment: ON | Dynamic Low-End Unmasking: ON", 40, 190, 500, 20, juce::Justification::left);
    g.drawText("[MASTER PROTOCOL] EBU R128 (-14 LUFS Target Profile)", 40, 210, 500, 20, juce::Justification::left);

    // ۴. فوتر پروژه
    g.setColour(juce::Colour(0xFF404E63));
    g.setFont(juce::Font(10.5f, juce::Font::plain));
    g.drawText("Engineered by Esmaeil Riahi // Zherfai Multi-Track Mesh Suite", 24, getHeight() - 25, getWidth() - 48, 16, juce::Justification::centred);
}

void ZherfaiAudioEngineEditor::resized()
{
    // قرارگیری کنترلرها در پنل پایینی
    stemSelector.setBounds (40, 318, 200, 32);
    autoGainButton.setBounds (260, 318, 190, 32);
    deMudButton.setBounds (460, 318, 160, 32);
    deEssButton.setBounds (260, 358, 190, 32);
}
