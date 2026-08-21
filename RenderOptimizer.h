#pragma once
#include "Config.h"
#include <chrono>
#include <vector>

class RenderOptimizer {
public:
    bool load();
    bool enable();
    bool disable();
    void onFrame();

private:
    RenderOptimizerConfig mConfig;
    std::vector<double> mFrameTimes;
    std::chrono::steady_clock::time_point mLastFrame{};
    bool mHasLastFrame = false;
    bool mThrottled = false;
    int mLowWindows = 0;
    int mRecoveryWindows = 0;

    double averageFps() const;
    void enterOptimizedState();
    void recoverGradually();
};
