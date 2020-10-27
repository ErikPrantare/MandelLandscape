#ifndef MANDELLANDSCAPE_TOACTIONS_TESTS_HPP
#define MANDELLANDSCAPE_TOACTIONS_TESTS_HPP

#include <map>
#include <set>

#include "glfwEnums.hpp"
#include "momentaryActionsMap.hpp"
#include "event.hpp"
#include "util.hpp"

namespace MomentaryActionsMapTests {

TEST_CASE("MomentaryActionsMap maps from glfwEvent", "[MomentaryActionsMap]")
{
    using namespace Input;

    MomentaryActionsMap map;
    map.add(Key::U, TriggerAction::IncreaseIterations);
    map.add(Key::U, TriggerAction::DecreaseIterations);

    SECTION("Mapping gives correct result")
    {
        Event event  = KeyDown{Key::U};
        auto actions = map(event);

        REQUIRE(actions.size() == 2);
        REQUIRE(util::contains(actions, {TriggerAction::IncreaseIterations}));
        REQUIRE(util::contains(actions, {TriggerAction::DecreaseIterations}));
    }

    SECTION("No given mapping -> empty container")
    {
        auto event   = KeyDown{Key::K};
        auto actions = map(event);
        REQUIRE(actions.size() == 0);
    }
}

}    // namespace MomentaryActionsMapTests

#endif