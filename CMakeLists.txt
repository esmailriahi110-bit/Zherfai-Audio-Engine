# ==============================================================================
# Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
# Project: Zherfai AI - Autonomous Multi-Node Mix & Master Engine
# ==============================================================================

cmake_minimum_required(VERSION 3.22)
project(ZherfaiAudioEngine VERSION 2.5.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(ZherfaiAudioEngine
    COMPANY_NAME "Zherfai"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Zhrf
    PLUGIN_CODE Zh01
    FORMATS VST3
    PRODUCT_NAME "Zherfai Channel Strip"
)

target_sources(ZherfaiAudioEngine
    PRIVATE
        Source/ZherfaiSharedMemory.h
        Source/PluginProcessor.h
        Source/PluginProcessor.cpp
        Source/PluginEditor.h
        Source/PluginEditor.cpp
)

target_compile_definitions(ZherfaiAudioEngine
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ZherfaiAudioEngine
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
