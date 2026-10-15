# Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
cmake_minimum_required(VERSION 3.22)

project(ZherfaiAudioEngine VERSION 2.5.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CRITICAL FIX: Disable JUCE helper applications and VST3 manifest validation that crash headless CI
set(JUCE_BUILD_HELPER_APPLICATIONS OFF CACHE BOOL "" FORCE)
set(JUCE_VST3_CAN_REPLACE_VST2 0)

# Fetch JUCE Framework (Pinned to stable v7.0.9)
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
)
FetchContent_MakeAvailable(juce)

# Define JUCE VST3 Plugin Target
juce_add_plugin(ZherfaiAudioEngine
    COMPANY_NAME "Zherfai AI"
    PLUGIN_NAME "Zherfai Channel Strip"
    BUNDLE_IDENTIFIER "com.zherfai.zherfaiudiocontrol"
    FORMATS VST3
    PRODUCT_NAME "ZherfaiAudioEngine"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE "Zhrf"
    PLUGIN_CODE "Zh01"
)

# Source Files Inclusion
target_sources(ZherfaiAudioEngine
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/ZherfaiSharedMemory.h
)

# Compiler Definitions & Optimizations
target_compile_definitions(ZherfaiAudioEngine
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link JUCE Modules
target_link_libraries(ZherfaiAudioEngine
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
