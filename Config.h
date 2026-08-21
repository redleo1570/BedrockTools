#pragma once
#include <string>

enum class OptimizerMode {
    Manual,
    Balanced,
    Adaptive,
    Aggressive
};

struct RenderOptimizerConfig {
    int version = 1;
    bool enabled = true;
    OptimizerMode mode = OptimizerMode::Adaptive;

    int targetFps = 60;
    int enterThreshold = 42;
    int recoverThreshold = 52;
    int sampleFrames = 45;
    int stableSeconds = 3;

    bool gradualRecovery = true;
    bool safeProbeOnly = true;
    bool probeRenderSignatures = true;
};
