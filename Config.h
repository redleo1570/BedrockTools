#pragma once
#include <string>
#include <pl/Config.hpp>

struct RenderOptimizerConfig {
    int version = 1;
    bool enabled = true;

    // 0 Manual, 1 Balanced, 2 Adaptive, 3 Aggressive
    int mode = 2;

    int targetFps = 60;
    int enterThreshold = 42;
    int recoverThreshold = 52;
    int sampleFrames = 45;
    int stableSeconds = 3;
    bool gradualRecovery = true;

    bool optimizeParticles = true;
    bool optimizeEntities = true;
    bool optimizeChunkUpdates = true;
    bool optimizeHud = false;
    bool optimizeAnimations = true;
};
