#pragma once
#include "Config.h"
#include <chrono>
#include <string>
#include <vector>
#include <cstdint>

namespace ll::mod { class NativeMod; }

class RenderOptimizer {
public:
    explicit RenderOptimizer(ll::mod::NativeMod* self);

    bool load();
    bool enable();
    bool disable();
    bool unload();

    void setConfig(const RenderOptimizerConfig& config);
    void onTick();

private:
    ll::mod::NativeMod* mSelf{};
    RenderOptimizerConfig mConfig{};

    std::vector<double> mSamples;
    std::chrono::steady_clock::time_point mLast{};
    bool mHasLast = false;

    bool mOptimized = false;
    int mLowWindows = 0;
    int mRecoveryWindows = 0;

    void registerMenu();
    void probeRendererSignatures();
    void subscribeFrameEvents();
    void unsubscribeFrameEvents();
    std::uint64_t mFrameSubscription = 0;

    double averageFps() const;
    void enterOptimizedState();
    void leaveOptimizedState();
    void applyConfigValue(const std::string& key, const std::string& value);
};
