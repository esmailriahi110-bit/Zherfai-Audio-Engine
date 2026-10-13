/*
  ==============================================================================
    Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
    Project: Zherfai AI - Autonomous Multi-Node Mix & Master Engine
    Module: Real-Time Lock-Free Inter-Process Communication (IPC)
  ==============================================================================
*/

#pragma once
#include <atomic>
#include <cstdint>

#define ZHERFAI_MAX_CHANNELS 32
#define ZHERFAI_SHM_NAME "Zherfai_Mesh_Matrix_Buffer"

enum StemType : uint32_t
{
    STEM_UNKNOWN = 0,
    STEM_SUB_LOW,         // Kick, 808, Sub-Bass (< 120 Hz focus)
    STEM_VOCAL_LEAD,      // Lead/Backing Vocal (Human Formants & High Presence)
    STEM_PERCUSSION,      // Snare, Claps, Hi-Hats (Fast Rise-Time transients)
    STEM_HARMONIC_PADS    // Instruments, Guitars, Synths (Sustained Mid-Range)
};

#pragma pack(push, 1)
struct ZherfaiChannelTelemetry
{
    std::atomic<bool> isActive{false};
    std::atomic<uint32_t> channelId{0};
    std::atomic<uint32_t> detectedStem{STEM_UNKNOWN};
    
    // Extracted Acoustic Features
    std::atomic<float> currentRmsDb{-100.0f};
    std::atomic<float> spectralCentroidHz{0.0f};
    std::atomic<float> zeroCrossingRate{0.0f};
    std::atomic<float> crestFactorDb{0.0f};
    
    // Dynamic Control Flags from Master Brain
    std::atomic<float> sidechainDuckGainDb{0.0f};
    std::atomic<float> dynamicDeMudGainDb{0.0f};
    std::atomic<float> vocalPocketCutDb{0.0f};
    std::atomic<bool> phaseInversionRequested{false};
};

struct ZherfaiMeshMatrix
{
    std::atomic<uint32_t> totalActiveNodes{0};
    std::atomic<uint32_t> activeGenreProfile{0}; // 0: HipHop, 1: Pop, 2: Rock, 3: EDM
    ZherfaiChannelTelemetry channels[ZHERFAI_MAX_CHANNELS];
};
#pragma pack(pop)
