/*
 * Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
 * Zherfai Shared Memory Mesh IPC & Telemetry Header (v2.5)
 */

#pragma once
#include <atomic>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

constexpr int ZHERFAI_MAX_CHANNELS = 32;
constexpr const char* ZHERFAI_SHM_NAME = "Local\\Zherfai_Mesh_Matrix_Buffer_v2";

enum ZherfaiStemType {
    STEM_UNKNOWN = 0,
    STEM_SUB_LOW = 1,       // Kick & Bass (Sub-Low < 120Hz)
    STEM_VOCAL_LEAD = 2,    // Lead Vocals (Formants 1kHz - 4kHz)
    STEM_PERCUSSION = 3,    // Drums & Transient Percussion (< 5ms rise)
    STEM_HARMONIC_PADS = 4  // Pads, Chords & Guitars
};

enum ZherfaiNodeRole {
    ROLE_AUTO_DETECT = 0,
    ROLE_CHANNEL_STRIP = 1,
    ROLE_MASTER_BRAIN = 2
};

struct alignas(64) ZherfaiChannelData {
    std::atomic<int> isActive{0};
    std::atomic<int> stemType{0};
    std::atomic<float> currentRmsDb{-100.0f};
    std::atomic<float> peakDb{-100.0f};
    std::atomic<float> sidechainDuckGainDb{0.0f};
    std::atomic<float> centroidHz{0.0f};
    char trackName[32]{"Unnamed"};
};

struct alignas(64) ZherfaiMasterGlobal {
    std::atomic<int> masterActive{0};
    std::atomic<int> selectedGenre{0}; // 0: Balanced, 1: HipHop, 2: Pop, 3: Rock, 4: EDM
    std::atomic<float> masterTargetLufs{-14.0f};
    std::atomic<float> masterTruePeakLimit{-1.0f};
    std::atomic<float> globalDuckingAmount{0.0f};
};

struct alignas(64) ZherfaiMeshMatrix {
    ZherfaiMasterGlobal masterState;
    ZherfaiChannelData channels[ZHERFAI_MAX_CHANNELS];
};
