# ==============================================================================
# Zherfai Master Brain Target (Architecture: Anti-Gravity)
# Developed by Esmaeil Riahi (UID: 2240088354)
# ==============================================================================
juce_add_plugin(ZherfaiMasterBrain
    COMPANY_NAME "Zherfai AI"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Zher
    PLUGIN_CODE Zmas
    FORMATS VST3
    PRODUCT_NAME "Zherfai Master Brain"
    VST3_AUTO_MANIFEST FALSE
)

target_sources(ZherfaiMasterBrain PRIVATE
    Source/MasterProcessor.cpp
    Source/MasterProcessor.h
    Source/MasterEditor.cpp
    Source/MasterEditor.h
    Source/ZherfaiSharedMemory.h
)

target_compile_definitions(ZherfaiMasterBrain PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ZherfaiMasterBrain PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce_recommended_config_flags
    juce_recommended_warning_flags
)
