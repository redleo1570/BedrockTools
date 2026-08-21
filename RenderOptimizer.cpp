#include "RenderOptimizer.h"
#include <algorithm>
#include <numeric>

bool RenderOptimizer::load() {
    mFrameTimes.reserve(120);
    return true;
}

bool RenderOptimizer::enable() {
    mThrottled = false;
    mLowWindows = 0;
    mRecoveryWindows = 0;
    mFrameTimes.clear();
    mHasLastFrame = false;
    return true;
}

bool RenderOptimizer::disable() {
    // V1 deliberately does not install risky renderer hooks.
    // This keeps the standalone package safe while the target-version
    // RenderDragon signatures are verified.
    mThrottled = false;
    return true;
}

double RenderOptimizer::averageFps() const {
    if (mFrameTimes.empty()) return 0.0;
    const size_t n = std::min<size_t>(mFrameTimes.size(),
        static_cast<size_t>(std::clamp(mConfig.sampleFrames, 15, 120)));
    const auto first = mFrameTimes.end() - static_cast<std::ptrdiff_t>(n);
    const double sum = std::accumulate(first, mFrameTimes.end(), 0.0);
    return sum > 0.0 ? static_cast<double>(n) / sum : 0.0;
}

void RenderOptimizer::enterOptimizedState() {
    mThrottled = true;
}

void RenderOptimizer::recoverGradually() {
    // Placeholder for verified renderer controls.
    // No speculative patching of RenderDragon is performed in V1.
    mThrottled = false;
}

void RenderOptimizer::onFrame() {
    if (!mConfig.enabled || mConfig.mode == 0) return;

    const auto now = std::chrono::steady_clock::now();
    if (!mHasLastFrame) {
        mLastFrame = now;
        mHasLastFrame = true;
        return;
    }

    const double dt = std::chrono::duration<double>(now - mLastFrame).count();
    mLastFrame = now;
    if (dt <= 0.001 || dt > 0.25) return;

    mFrameTimes.push_back(dt);
    if (mFrameTimes.size() > 120) mFrameTimes.erase(mFrameTimes.begin());
    if (mFrameTimes.size() < static_cast<size_t>(std::clamp(mConfig.sampleFrames,15,120))) return;

    const double fps = averageFps();
    if (!mThrottled && fps < mConfig.enterThreshold) {
        if (++mLowWindows >= 2) enterOptimizedState();
    } else if (mThrottled && fps >= mConfig.recoverThreshold) {
        if (++mRecoveryWindows >= std::max(1, mConfig.stableSeconds * 2)) {
            recoverGradually();
            mRecoveryWindows = 0;
        }
    } else if (fps >= mConfig.enterThreshold) {
        mLowWindows = 0;
        mRecoveryWindows = 0;
    }
}
