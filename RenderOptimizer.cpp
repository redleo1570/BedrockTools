#include "RenderOptimizer.h"

#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include <pl/memory/Signature.hpp>
#include <bedrocktools/Api.hpp>
#include <bedrocktools/memory/Signatures.hpp>
#include <bedrocktools/events/Event.hpp>

#include <algorithm>
#include <cstdlib>
#include <string>

RenderOptimizer::RenderOptimizer(ll::mod::NativeMod* self) : mSelf(self) {}

bool RenderOptimizer::load() {
    if (!mSelf) return false;
    registerMenu();
    mSamples.reserve(120);
    mSelf->getLogger().info("Render Optimizer {} loaded", mSelf->getVersion());
    return true;
}

bool RenderOptimizer::enable() {
    if (!mSelf) return false;
    mOptimized = false;
    mLowWindows = 0;
    mRecoveryWindows = 0;
    mSamples.clear();
    mHasLast = false;

    if (mConfig.probeRenderSignatures) {
        probeRendererSignatures();
    }
    subscribeFrameEvents();
    return true;
}

bool RenderOptimizer::disable() {
    // V2 deliberately restores its own state and does not leave speculative
    // renderer patches installed.
    unsubscribeFrameEvents();
    mOptimized = false;
    mSamples.clear();
    return true;
}

bool RenderOptimizer::unload() {
    mOptimized = false;
    mSamples.clear();
    return true;
}

void RenderOptimizer::registerMenu() {
    using namespace pl::modmenu;

    ModuleBuilder builder("render_optimizer", "Render Optimizer");
    builder
        .description("Otimizador adaptativo de renderizacao com controle de estabilidade.")
        .modId(mSelf->getId())
        .defaultEnabled(true)
        .config("mode", "Modo", ConfigType::Radio, "Adaptive", "Manual,Balanced,Adaptive,Aggressive")
        .config("targetFps", "FPS alvo", ConfigType::SliderInt, "60", "20", "120")
        .config("enterThreshold", "Entrar abaixo de", ConfigType::SliderInt, "42", "20", "120")
        .config("recoverThreshold", "Recuperar acima de", ConfigType::SliderInt, "52", "25", "144")
        .config("sampleFrames", "Amostras", ConfigType::SliderInt, "45", "15", "120")
        .config("stableSeconds", "Estabilidade", ConfigType::SliderInt, "3", "1", "10")
        .config("gradualRecovery", "Recuperacao gradual", ConfigType::Toggle, "true")
        .config("safeProbeOnly", "Modo seguro", ConfigType::Toggle, "true")
        .onToggle([this](std::string_view, bool enabled) {
            if (enabled) enable(); else disable();
        })
        .onConfigChanged([this](std::string_view, std::string_view key, std::string_view value) {
            applyConfigValue(std::string(key), std::string(value));
        })
        .registerModule();
}

void RenderOptimizer::applyConfigValue(const std::string& key, const std::string& value) {
    if (key == "mode") {
        if (value == "Manual") mConfig.mode = OptimizerMode::Manual;
        else if (value == "Balanced") mConfig.mode = OptimizerMode::Balanced;
        else if (value == "Aggressive") mConfig.mode = OptimizerMode::Aggressive;
        else mConfig.mode = OptimizerMode::Adaptive;
    } else if (key == "targetFps") mConfig.targetFps = std::clamp(std::atoi(value.c_str()), 20, 120);
    else if (key == "enterThreshold") mConfig.enterThreshold = std::clamp(std::atoi(value.c_str()), 20, 120);
    else if (key == "recoverThreshold") mConfig.recoverThreshold = std::clamp(std::atoi(value.c_str()), 25, 144);
    else if (key == "sampleFrames") mConfig.sampleFrames = std::clamp(std::atoi(value.c_str()), 15, 120);
    else if (key == "stableSeconds") mConfig.stableSeconds = std::clamp(std::atoi(value.c_str()), 1, 10);
    else if (key == "gradualRecovery") mConfig.gradualRecovery = (value == "true" || value == "1");
    else if (key == "safeProbeOnly") mConfig.safeProbeOnly = (value == "true" || value == "1");
}

void RenderOptimizer::probeRendererSignatures() {
    if (!mSelf) return;

    const auto* api = bedrocktools::api::find();
    if (!api) {
        mSelf->getLogger().warn("BedrockTools API v1 not available; renderer probe skipped.");
        return;
    }

    const auto renderLevel =
        bedrocktools::api::resolve(bedrocktools::memory::SignatureId::RenderLevel, api);
    const auto tessBegin =
        bedrocktools::api::resolve(bedrocktools::memory::SignatureId::TessellatorBegin, api);
    const auto meshImmediate =
        bedrocktools::api::resolve(
            bedrocktools::memory::SignatureId::MeshHelpersRenderMeshImmediately, api);
    const auto dirty =
        bedrocktools::api::resolve(
            bedrocktools::memory::SignatureId::RenderChunkCoordinatorSetAllDirty, api);

    mSelf->getLogger().info(
        "Renderer probe: RenderLevel=0x{:x}, TessellatorBegin=0x{:x}, "
        "MeshImmediate=0x{:x}, ChunkDirty=0x{:x}",
        static_cast<std::uintptr_t>(renderLevel),
        static_cast<std::uintptr_t>(tessBegin),
        static_cast<std::uintptr_t>(meshImmediate),
        static_cast<std::uintptr_t>(dirty));
}

void RenderOptimizer::subscribeFrameEvents() {
    if (mFrameSubscription) return;

    const auto* api = bedrocktools::api::find();
    if (!api || !api->subscribe) {
        if (mSelf) mSelf->getLogger().warn("BedrockTools Frame event API unavailable.");
        return;
    }

    using namespace bedrocktools::events;
    mFrameSubscription = api->subscribe(
        EventType::Frame,
        EventPriority::Late,
        [](EventType, void* payload, void* userData) {
            (void)payload;
            auto* self = static_cast<RenderOptimizer*>(userData);
            if (self) self->onTick();
        },
        this);

    if (mSelf) {
        mSelf->getLogger().info("Frame event subscription: {}", mFrameSubscription ? "OK" : "FAILED");
    }
}

void RenderOptimizer::unsubscribeFrameEvents() {
    if (!mFrameSubscription) return;

    const auto* api = bedrocktools::api::find();
    if (api && api->unsubscribe) {
        api->unsubscribe(mFrameSubscription);
    }
    mFrameSubscription = 0;
}

double RenderOptimizer::averageFps() const {
    if (mSamples.empty()) return 0.0;
    const size_t n = std::min<size_t>(mSamples.size(),
        static_cast<size_t>(std::clamp(mConfig.sampleFrames, 15, 120)));
    const auto first = mSamples.end() - static_cast<std::ptrdiff_t>(n);
    double total = 0.0;
    for (auto it = first; it != mSamples.end(); ++it) total += *it;
    return total > 0.0 ? static_cast<double>(n) / total : 0.0;
}

void RenderOptimizer::enterOptimizedState() {
    // V2 does not patch RenderDragon. It only changes optimizer state.
    // V3 will install individually verified hooks.
    mOptimized = true;
    if (mSelf) mSelf->getLogger().info("Adaptive state -> OPTIMIZED (safe monitor mode)");
}

void RenderOptimizer::leaveOptimizedState() {
    mOptimized = false;
    if (mSelf) mSelf->getLogger().info("Adaptive state -> NORMAL (safe monitor mode)");
}

void RenderOptimizer::onTick() {
    if (!mConfig.enabled || mConfig.mode == OptimizerMode::Manual) return;

    const auto now = std::chrono::steady_clock::now();
    if (!mHasLast) {
        mLast = now;
        mHasLast = true;
        return;
    }

    const double dt = std::chrono::duration<double>(now - mLast).count();
    mLast = now;
    if (dt <= 0.001 || dt > 0.25) return;

    mSamples.push_back(dt);
    if (mSamples.size() > 120) mSamples.erase(mSamples.begin());

    const size_t required = static_cast<size_t>(
        std::clamp(mConfig.sampleFrames, 15, 120));
    if (mSamples.size() < required) return;

    const double fps = averageFps();
    const int enter = mConfig.enterThreshold;
    const int recover = std::max(mConfig.recoverThreshold, enter + 5);

    if (!mOptimized && fps < enter) {
        if (++mLowWindows >= 2) {
            enterOptimizedState();
            mLowWindows = 0;
        }
    } else if (mOptimized && fps >= recover) {
        if (++mRecoveryWindows >= std::max(1, mConfig.stableSeconds * 2)) {
            leaveOptimizedState();
            mRecoveryWindows = 0;
        }
    } else if (fps >= enter) {
        mLowWindows = 0;
        mRecoveryWindows = 0;
    }
}
