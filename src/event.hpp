#ifndef MANDELLANDSCAPE_EVENT_HPP
#define MANDELLANDSCAPE_EVENT_HPP

#include <variant>

#include "glfwEnums.hpp"

struct KeyDown {
    Input::Key code;
    int const mods;
};

struct KeyUp {
    Input::Key code;
    int const mods;
};

struct MouseMove {
    double const x;
    double const y;
    double const dx;
    double const dy;
};

struct MouseButtonDown {
    int const button;
};

struct MouseButtonUp {
    int const button;
};

using Event = std::
        variant<KeyUp, KeyDown, MouseMove, MouseButtonUp, MouseButtonDown>;

bool
operator<(MouseMove a, MouseMove b);
bool
operator<(MouseButtonUp a, MouseButtonUp b);
bool
operator<(MouseButtonDown a, MouseButtonDown b);

#endif
