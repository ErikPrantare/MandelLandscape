#include "walkController.hpp"

#include <algorithm>

#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>

#include "utils.hpp"

auto
WalkController::update(Player* const player, double const dt) -> void
{
    auto constexpr pi = glm::pi<double>();

    player->position += dt * movementSpeed * m_direction;
    player->lookAtOffset += m_dLookAtOffset;
    player->lookAtOffset.y =
            std::clamp(player->lookAtOffset.y, -pi / 2 + 0.001, pi / 2 - 0.001);
    m_dLookAtOffset = glm::dvec2{0.0, 0.0};
}

auto
WalkController::handleEvent(Event const& event) -> void
{
    std::visit(
            util::Overload{
                    [this](KeyDown key) {
                        switch(key.code) {
                        case GLFW_KEY_W: {
                            m_direction.z += -1.0;
                        } break;
                        case GLFW_KEY_A: {
                            m_direction.x += -1.0;
                        } break;
                        case GLFW_KEY_S: {
                            m_direction.z += 1.0;
                        } break;
                        case GLFW_KEY_D: {
                            m_direction.x += 1.0;
                        } break;
                        }
                    },

                    [this](KeyUp key) {
                        switch(key.code) {
                        case GLFW_KEY_W: {
                            m_direction.z += 1.0;
                        } break;
                        case GLFW_KEY_A: {
                            m_direction.x += 1.0;
                        } break;
                        case GLFW_KEY_S: {
                            m_direction.z += -1.0;
                        } break;
                        case GLFW_KEY_D: {
                            m_direction.x += -1.0;
                        } break;
                        }
                    },

                    [this](MouseMove mouse) {
                        m_dLookAtOffset.x += util::pixelsToAngle(mouse.dx);
                        m_dLookAtOffset.y += util::pixelsToAngle(mouse.dy);
                    },

                    // default
                    util::unaryNOP},
            event);
}
