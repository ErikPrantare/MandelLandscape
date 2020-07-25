#ifndef MANDELLANDSCAPE_SETTINGS_HPP
#define MANDELLANDSCAPE_SETTINGS_HPP

#include "mandelTypeTraits.hpp"

namespace Settings {
template<
        typename T,
        int _uid,
        typename = Require<std::is_move_constructible_v<T>>>
struct Setting {
    using Type               = T;
    static constexpr int uid = _uid;
};

template<typename T>
using RequireSetting = RequireSame<T, Setting<typename T::Type, T::uid>>;

// Only create settings here. __LINE__ is to give each setting a unique id
using WindowHeight      = Setting<int, __LINE__>;
using WindowWidth       = Setting<int, __LINE__>;
using ClippingPlaneNear = Setting<double, __LINE__>;
using ClippingPlaneFar  = Setting<double, __LINE__>;
using FOV               = Setting<double, __LINE__>;
using UseDeepShader     = Setting<bool, __LINE__>;
using Iterations        = Setting<int, __LINE__>;
}    // namespace Settings

#endif    // MANDELLANDSCAPE_SETTINGS_HPP
