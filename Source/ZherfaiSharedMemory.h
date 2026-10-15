/**
 * ==============================================================================
 * Zherfai Audio Engine - Multi-Node Mesh Topology IPC
 * Developed by Esmaeil Riahi (UID: 2240088354) | Architecture: Anti-Gravity
 * ==============================================================================
 */

#pragma once
#include <cstdint>

#define ZHERFAI_MAX_CHANNELS 32
#define ZHERFAI_SHARED_MEM_NAME "Local\\Zherfai_Mesh_Matrix_Buffer_v2"

enum ZherfaiStemType : uint32_t {
    STEM_UNKNOWN = 0,
    STEM_SUB_LOW = 1,        // Kick & Bass
    STEM_VOCAL_LEAD = 2,     // Lead Vocals & Voiceover
    STEM_PERCUSSION = 3,     // Snare, Claps, Hi-Hats, Tops
    STEM_HARMONIC_PADS = 4   // Guitars, Synths, Keys, Orchestral
};

enum ZherfaiGenreProfile : uint32_t {
    GENRE_BALANCED = 0,
    GENRE_HIPHOP_TRAP = 1,
    GENRE_POP_VOCAL = 2,
    GENRE_ROCK_ACOUSTIC = 3,
    GENRE_EDM = 4
};

#pragma pack(push, 8)
struct alignas(64) ZherfaiChannelTelemetry {
    uint32_t isActive;
    uint32_t channelId;
    ZherfaiStemType stemType;
    float currentRmsDb;
    float currentPeakDb;
    float spectralCentroid;
    float crestFactor;
    float sidechainDuckGainDb; // Attenuation instructed by Master
    char channelName[32];
};

struct alignas(64) ZherfaiMasterMatrix {
    uint32_t masterActive;
    ZherfaiGenreProfile activeGenre;
    float targetIntegratedLufs;
    float truePeakCeilingDb;
    uint32_t connectedChannelsCount;
    ZherfaiChannelTelemetry channels[ZHERFAI_MAX_CHANNELS];
};
#pragma pack(pop)
