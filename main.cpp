#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include <pl/Config.hpp>
#include "mod/RenderOptimizer.h"

class RenderOptimizerMod final : public pl::Mod {
public:
    RenderOptimizerMod() : pl::Mod() {}

    bool load() override {
        getSelf().getLogger().info("Render Optimizer loading");
        return mOptimizer.load();
    }

    bool enable() override {
        getSelf().getLogger().info("Render Optimizer enabled");
        return mOptimizer.enable();
    }

    bool disable() override {
        getSelf().getLogger().info("Render Optimizer disabled");
        return mOptimizer.disable();
    }

    bool unload() override {
        return true;
    }

private:
    RenderOptimizer mOptimizer;
};

PL_REGISTER_MOD(RenderOptimizerMod)
