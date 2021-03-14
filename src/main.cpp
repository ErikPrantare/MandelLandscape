/* Mandelscape
 * Copyright (C) 2020-2021 Erik Präntare
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <tuple>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <nfd.hpp>

#include "util.hpp"
#include "camera.hpp"
#include "terrain.hpp"
#include "window.hpp"
#include "player.hpp"
#include "playerHelper.hpp"
#include "walkController.hpp"
#include "autoController.hpp"
#include "metaController.hpp"
#include "persistentActionMap.hpp"
#include "momentaryActionsMap.hpp"
#include "shaderController.hpp"

void
renderScene(
        Player const& player,
        glm::ivec2 viewSize,
        ShaderProgram* program,
        double dt);

auto
initControls() -> std::pair<MomentaryActionsMap, PersistentActionMap>;

auto
initControlsDvorak() -> std::pair<MomentaryActionsMap, PersistentActionMap>;

auto
main(int argc, char* argv[]) -> int
try {
    NFD_Init();

    nfdchar_t* outPath;
    nfdfilteritem_t filterItem[2] = {
            {"Source code", "c,cpp,cc"},
            {"Headers", "h,hpp"}};
    nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, NULL);
    (void)result;
    auto const args = std::vector(argv, argv + argc);

    auto window = Window({1368, 768});

    auto terrain = Terrain();
    auto player  = Player();

    auto [momentaryMap, persistentMap] = [&args] {
        if(args.size() == 2 && args[1] == std::string("--dvorak")) {
            return initControlsDvorak();
        }

        return initControls();
    }();

    auto autoControllHeightFunc = [&terrain](glm::dvec2 x) {
        return terrain.heightAt(x);
    };

    auto metaController = MetaController{
            std::make_unique<WalkController>(),
            std::make_unique<AutoController>(autoControllHeightFunc)};

    auto shaderProgram = ShaderProgram();
    shaderProgram.bindAttributeLocation("pos", Mesh::vertexLocation);
    shaderProgram.bindAttributeLocation("val", Terrain::colorLocation);

    auto shaderController = ShaderController(&shaderProgram);

    auto time            = 0.0;
    double lastTimepoint = glfwGetTime();
    while(window.update()) {
        double const currentTimepoint = glfwGetTime();
        double const dt               = currentTimepoint - lastTimepoint;
        lastTimepoint                 = currentTimepoint;

        while(auto const eventOpt = window.nextEvent()) {
            auto const event = eventOpt.value();

            persistentMap.updateState(event);

            for(auto const& action : momentaryMap(event)) {
                window.handleMomentaryAction(action);
                terrain.handleMomentaryAction(action);
                metaController.handleMomentaryAction(action);
                shaderController.handleMomentaryAction(action);

                if(action == MomentaryAction{TriggerAction::Save}) {
                    std::ofstream out("save.lua");
                    out << serialize(player);
                }
                else if(action == MomentaryAction{TriggerAction::Load}) {
                    std::ifstream in("save.lua");
                    player = deserialize(util::getContents(in));
                }
            }
        }

        if(!window.paused()) {
            time += dt;

            metaController.updateState(persistentMap);
            shaderController.updateState(persistentMap, dt);

            auto const pos = PlayerHelper(player).truePosition();
            terrain.updateMesh(pos.x, pos.z, 1.0 / player.scale);
            PlayerHelper(player).updateOffset(terrain.offset());
            player.position.y = terrain.heightAt({pos.x, pos.z});
            metaController.update(&player, dt);

            // Do this after .update, as autozoom is dependent on position.y
            player.position.y *= shaderController.yScale();
        }

        shaderController.update(&shaderProgram);
        shaderProgram.setUniformFloat("time", time);
        renderScene(player, window.size(), &shaderProgram, dt);
        terrain.render(&shaderProgram);
    }

    return 0;
}
catch(std::runtime_error const& e) {
    std::cerr << e.what() << std::endl;

    return 1;
}

void
renderScene(
        Player const& player,
        glm::ivec2 const viewSize,
        ShaderProgram* const program,
        double dt)
{
    glm::dvec3 cameraPosition = player.position;
    cameraPosition.y += player.scale;
    static util::LowPassFilter filteredHeight(cameraPosition.y, 0.01);
    cameraPosition.y = filteredHeight(cameraPosition.y, dt);

    // + util::pi, because -z is regarded as the default lookAt forward
    auto const lookAt = glm::rotate(
                                glm::dmat4(1.0),
                                player.lookAtOffset.x + util::pi,
                                {0.0, 1.0, 0.0})
                        * glm::rotate(
                                glm::dmat4(1.0),
                                player.lookAtOffset.y,
                                {1.0, 0.0, 0.0})
                        * glm::dvec4(0.0, 0.0, 1.0, 0.0);

    auto const camera = Camera(cameraPosition, lookAt, viewSize, player.scale);
    program->setUniformMatrix4("cameraSpace", camera.cameraSpace());
    program->setUniformMatrix4("projection", camera.projection());
}

[[nodiscard]] auto
initControls() -> std::pair<MomentaryActionsMap, PersistentActionMap>
{
    auto momentaryMap = MomentaryActionsMap();
    momentaryMap.add(Input::Key::C, TriggerAction::ToggleAutoWalk);
    momentaryMap.add(Input::Key::O, TriggerAction::ToggleAutoZoom);
    momentaryMap.add(Input::Key::K, TriggerAction::IncreaseIterations);
    momentaryMap.add(Input::Key::J, TriggerAction::DecreaseIterations);
    momentaryMap.add(Input::Key::F, TriggerAction::ToggleFastMode);
    momentaryMap.add(Input::Key::H, TriggerAction::SwitchShader);
    momentaryMap.add(Input::Key::P, TriggerAction::TogglePause);
    momentaryMap.add(Input::Key::X, TriggerAction::TakeScreenshot);
    momentaryMap.add(Input::Key::Q, TriggerAction::CloseWindow);
    momentaryMap.add(Input::Key::ESCAPE, TriggerAction::CloseWindow);

    auto persistentMap = PersistentActionMap();
    persistentMap.add(Input::Key::W, PersistentAction::MoveForwards);
    persistentMap.add(Input::Key::S, PersistentAction::MoveBackwards);
    persistentMap.add(Input::Key::A, PersistentAction::MoveLeft);
    persistentMap.add(Input::Key::D, PersistentAction::MoveRight);
    persistentMap.add(Input::MouseButton::LEFT, PersistentAction::ZoomIn);
    persistentMap.add(Input::MouseButton::RIGHT, PersistentAction::ZoomOut);
    persistentMap.add(Input::Key::UP, PersistentAction::IncreaseParam);
    persistentMap.add(Input::Key::DOWN, PersistentAction::DecreaseParam);
    persistentMap.add(Input::Key::KEY_1, PersistentAction::ChangeFrequency);
    persistentMap.add(Input::Key::KEY_2, PersistentAction::ChangeTotalOffset);
    persistentMap.add(Input::Key::KEY_3, PersistentAction::ChangeGreenOffset);
    persistentMap.add(Input::Key::KEY_4, PersistentAction::ChangeBlueOffset);
    persistentMap.add(Input::Key::KEY_5, PersistentAction::ChangeYScale);

    return {momentaryMap, persistentMap};
}

[[nodiscard]] auto
initControlsDvorak() -> std::pair<MomentaryActionsMap, PersistentActionMap>
{
    auto momentaryMap = MomentaryActionsMap();
    momentaryMap.add(Input::Key::J, TriggerAction::ToggleAutoWalk);
    momentaryMap.add(Input::Key::R, TriggerAction::ToggleAutoZoom);
    momentaryMap.add(Input::Key::T, TriggerAction::IncreaseIterations);
    momentaryMap.add(Input::Key::H, TriggerAction::DecreaseIterations);
    momentaryMap.add(Input::Key::F, TriggerAction::ToggleFastMode);
    momentaryMap.add(Input::Key::D, TriggerAction::SwitchShader);
    momentaryMap.add(Input::Key::P, TriggerAction::TogglePause);
    momentaryMap.add(Input::Key::X, TriggerAction::TakeScreenshot);
    momentaryMap.add(Input::Key::Q, TriggerAction::CloseWindow);
    momentaryMap.add(Input::Key::ESCAPE, TriggerAction::CloseWindow);
    momentaryMap.add(Input::Key::KEY_6, TriggerAction::Save);
    momentaryMap.add(Input::Key::KEY_7, TriggerAction::Load);

    auto persistentMap = PersistentActionMap();
    persistentMap.add(Input::Key::COMMA, PersistentAction::MoveForwards);
    persistentMap.add(Input::Key::O, PersistentAction::MoveBackwards);
    persistentMap.add(Input::Key::A, PersistentAction::MoveLeft);
    persistentMap.add(Input::Key::E, PersistentAction::MoveRight);
    persistentMap.add(Input::MouseButton::LEFT, PersistentAction::ZoomIn);
    persistentMap.add(Input::MouseButton::RIGHT, PersistentAction::ZoomOut);
    persistentMap.add(Input::Key::UP, PersistentAction::IncreaseParam);
    persistentMap.add(Input::Key::DOWN, PersistentAction::DecreaseParam);
    persistentMap.add(Input::Key::KEY_1, PersistentAction::ChangeFrequency);
    persistentMap.add(Input::Key::KEY_2, PersistentAction::ChangeTotalOffset);
    persistentMap.add(Input::Key::KEY_3, PersistentAction::ChangeGreenOffset);
    persistentMap.add(Input::Key::KEY_4, PersistentAction::ChangeBlueOffset);
    persistentMap.add(Input::Key::KEY_5, PersistentAction::ChangeYScale);

    return {momentaryMap, persistentMap};
}
