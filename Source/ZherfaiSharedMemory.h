#pragma once
#include <cstdint>

#define ZHERFAI_SHM_NAME "Local\\Zherfai_Mesh_Matrix_Buffer_v2"
#define ZHERFAI_MAX_CHANNELS 32

enum ZherfaiStemType {
    STEM_UNKNOWN = 0,
    STEM_SUB_LOW,      // کیک و ساب‌بیس
    STEM_VOCAL_LEAD,   // وکال اصلی
    STEM_PERCUSSION,   // اسنیر، های‌هت و پرکاشن
    STEM_HARMONIC_PADS // گیتار، پیانو، سینت و پدها
};

enum ZherfaiGenreProfile {
    GENRE_BALANCED = 0,
    GENRE_HIPHOP_TRAP,
    GENRE_POP_VOCAL,
    GENRE_ROCK_ACOUSTIC,
    GENRE_EDM
};

struct ZherfaiChannelTelemetry {
    volatile int isActive;
    volatile int channelId;
    volatile int stemType;
    float currentRmsDb;
    float spectralCentroid;
    float crestFactor;
    float sidechainDuckGainDb;
    float dynamicDeMudGainDb;
    float vocalPocketCutDb;
    volatile int phaseInversionRequested;
};

struct ZherfaiMeshMatrix {
    volatile int totalActiveNodes;
    volatile int activeGenre;
    ZherfaiChannelTelemetry channels[ZHERFAI_MAX_CHANNELS];
};
