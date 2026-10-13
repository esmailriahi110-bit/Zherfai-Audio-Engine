/*
  ==============================================================================
    Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
    Project: Zherfai AI - Autonomous Multi-Node Mix & Master Engine
    Component: Mesh Topology & Shared Memory Declarations
  ==============================================================================
*/

#pragma once

#include <atomic>
#include <cstdint>

#define ZHERFAI_SHM_NAME "Local\\Zherfai_Mesh_Matrix_Buffer"
#define ZHERFAI_MAX_CHANNELS 32

enum StemType : int32_t
{
    STEM_UNKNOWN = 0,
    STEM_SUB_LOW = 1,
    STEM_VOCAL_LEAD = 2,
    STEM_PERCUSSION = 3,
    STEM_HARMONIC_PADS = 4
};

enum GenreProfile : int32_t
{
    GENRE_BALANCED = 0,
    GENRE_HIPHOP_TRAP = 1,
    GENRE_POP_VOCAL = 2,
    GENRE_ROCK_ACOUSTIC = 3,
    GENRE_EDM = 4
};

// Align to 64-byte boundary to guarantee atomic operations validity and prevent false sharing
struct alignas(64) ZherfaiChannelTelemetry
{
    std::atomic<bool> isActive{false};
    std::atomic<int32_t> channelId{0};
    std::atomic<StemType> detectedStem{STEM_UNKNOWN};
    
    std::atomic<float> currentRmsDb{-100.0f};
    std::atomic<float> spectralCentroidHz{0.0f};
    std::atomic<float> zeroCrossingRate{0.0f};
    std::atomic<float> crestFactorDb{0.0f};

    // Master Brain Real-Time Directives
    std::atomic<float> sidechainDuckGainDb{0.0f};
    std::atomic<float> dynamicDeMudGainDb{0.0f};
    std::atomic<float> vocalPocketCutDb{0.0f};
    std::atomic<bool> phaseInversionRequested{false};
};

struct alignas(64) ZherfaiMeshMatrix
{
    std::atomic<int32_t> totalActiveNodes{0};
    std::atomic<GenreProfile> activeGenreProfile{GENRE_BALANCED};
    ZherfaiChannelTelemetry channels[ZHERFAI_MAX_CHANNELS];
};
