#include "event.hpp"
#include "metaController.hpp"
#include "utils.hpp"
#include "camera.hpp"
#include "terrain.hpp"
#include "config.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "player.hpp"
#include "walkController.hpp"
#include "autoController.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <variant>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>

long double constexpr pi = glm::pi<long double>();

void
renderScene(
        Terrain& terrain,
        const Player& player,
        Config const& config,
        double dt);

Config
initConfig();

int
main(int, char**)
{
    auto config = initConfig();
    auto window = Window(config);

    auto terrain = Terrain();
    auto player  = Player();

    auto autoControllHeightFunc = [&](glm::dvec2 x) {
        return terrain.heightAt(x);
    };
    auto metacontroller = Metacontroller{
            WalkController{},
            AutoController{autoControllHeightFunc}};

    double lastTimepoint = glfwGetTime();
    while(window.update()) {
        const double currentTimepoint = glfwGetTime();
        const double dt               = currentTimepoint - lastTimepoint;
        lastTimepoint                 = currentTimepoint;

        while(auto eventOpt = window.nextEvent()) {
            auto const event = eventOpt.value();

            terrain.handleEvent(event);
            metacontroller.handleEvent(event);
            window.handleEvent(event);
        }

        auto pos = player.position + player.positionOffset;
        auto terrainOffset =
                terrain.updateMesh(pos.x, pos.z, 1.0 / player.scale);
        auto dOffset          = terrainOffset - player.positionOffset;
        player.positionOffset = terrainOffset;
        player.position -= dOffset;
        player.position.y = terrain.heightAt({pos.x, pos.z});
        metacontroller.update(player, dt);

        renderScene(terrain, player, config, dt);
    }

    return 0;
}

void
renderScene(
        Terrain& terrain,
        const Player& player,
        Config const& config,
        double dt)
{
    auto camera = Camera(config);
    camera.setScale(player.scale);

    glm::vec3 cameraPosition = player.position;
    cameraPosition.y += player.scale;

    static util::LowPassFilter filteredHeight(cameraPosition.y, 0.01);
    cameraPosition.y = filteredHeight(cameraPosition.y, dt);

    camera.setPosition(cameraPosition);

    // + double(pi), because -z is regarded as the default lookAt forward
    camera.lookAt(
            glm::rotate(
                    glm::dmat4(1.0),
                    player.lookAtOffset.x + double(pi),
                    {0.0, 1.0, 0.0})
            * glm::rotate(
                    glm::dmat4(1.0),
                    player.lookAtOffset.y,
                    {1.0, 0.0, 0.0})
            * glm::dvec4(0.0, 0.0, 1.0, 0.0));

    auto& program = terrain.shaderProgram();
    program.setUniformMatrix4("cameraSpace", camera.cameraSpace());
    program.setUniformMatrix4("projection", camera.projection());

    terrain.render();
}

Config
initConfig()
{
    Config conf;
    conf.set<Settings::WindowWidth>(1366);
    conf.set<Settings::WindowHeight>(768);
    conf.set<Settings::ClippingPlaneNear>(0.01);
    conf.set<Settings::ClippingPlaneFar>(150.0);
    conf.set<Settings::FOV>(pi / 2);
    conf.set<Settings::UseDeepShader>(false);
    conf.set<Settings::Iterations>(100);

    return conf;
}
