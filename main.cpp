#include <pl/Mod.hpp>
#include "mod/RenderOptimizer.h"

class RenderOptimizerMod final {
public:
    RenderOptimizerMod() : mSelf(*ll::mod::NativeMod::current()), mOptimizer(&mSelf) {}

    bool load() { return mOptimizer.load(); }
    bool enable() { return mOptimizer.enable(); }
    bool disable() { return mOptimizer.disable(); }
    bool unload() { return mOptimizer.unload(); }

private:
    ll::mod::NativeMod& mSelf;
    RenderOptimizer mOptimizer;
};

PL_REGISTER_MOD(RenderOptimizerMod, RenderOptimizerMod())
