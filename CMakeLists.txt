# ==============================================================================
# Zherfai Audio Engine - Autonomous VST3 Suite
# Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
# ==============================================================================

cmake_minimum_required(VERSION 3.22)
project(ZherfaiAudioEngine VERSION 2.5.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Bypass UI helper apps in headless CI environments
set(JUCE_BUILD_HELPER_APPLICATIONS OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(ZherfaiAudioEngine
    COMPANY_NAME "Zherfai AI"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_NAME "Zherfai Channel Strip"
    PRODUCT_NAME "Zherfai Channel Strip"
    BUNDLE_ID "com.zherfai.zherfaiudiocontrol"
    FORMATS VST3
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_CODE Zh01
    PLUGIN_MANUFACTURER_CODE Zhrf
    VST3_AUTO_MANIFEST FALSE
)

target_sources(ZherfaiAudioEngine PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/ZherfaiSharedMemory.h
)

target_compile_definitions(ZherfaiAudioEngine PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISABLE_VST3_MANIFEST_CHECK=1
)

target_link_libraries(ZherfaiAudioEngine PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_core
)
