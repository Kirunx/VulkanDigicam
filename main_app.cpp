#include "main_app.hpp"
#include "simple_render_system.hpp"
#include "vp_camera.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <iostream>
#include <stdexcept>

namespace vp {

MainApp::MainApp() {
    loadGameObjects();
}

MainApp::~MainApp() { }
void MainApp::run() {
    SimpleRenderSystem simpleRenderSystme { vpDevice, vpRenderer.getSwapChainRenderPass() };
    VpCamera camera { };
    // camera.setViewDirection(glm::vec3 { 0.f }, glm::vec3 { 0.5f, 0.f, 1.f });

    while (!vpWindow.shouldClose()) {
        glfwPollEvents();
        float aspect = vpRenderer.getAspectRation();

        // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
        if (auto commandBuffer = vpRenderer.beginFrame()) {
            vpRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystme.renderGameObjects(commandBuffer, gameObjects, camera);
            vpRenderer.endSwapChainRenderPass(commandBuffer);
            vpRenderer.endFrame();
        }
    }
    vkDeviceWaitIdle(vpDevice.device());
}

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<VpModel> createCubeModel(VpDevice& device, glm::vec3 offset) {
    std::vector<VpModel::Vertex> vertices {

        // left face (white)
        { { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, .5f }, { .9f, .9f, .9f } },
        { { -.5f, -.5f, .5f }, { .9f, .9f, .9f } },
        { { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, .5f }, { .9f, .9f, .9f } },

        // right face (yellow)
        { { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .8f, .1f } },
        { { .5f, -.5f, .5f }, { .8f, .8f, .1f } },
        { { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .8f, .1f } },

        // top face (orange, remember y axis points down)
        { { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, .5f }, { .9f, .6f, .1f } },
        { { -.5f, -.5f, .5f }, { .9f, .6f, .1f } },
        { { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, .5f }, { .9f, .6f, .1f } },

        // bottom face (red)
        { { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .1f, .1f } },
        { { -.5f, .5f, .5f }, { .8f, .1f, .1f } },
        { { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .1f, .1f } },

        // nose face (blue)
        { { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },
        { { -.5f, .5f, 0.5f }, { .1f, .1f, .8f } },
        { { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },

        // tail face (green)
        { { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },
        { { -.5f, .5f, -0.5f }, { .1f, .8f, .1f } },
        { { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },

    };
    for (auto& v : vertices) {
        v.position += offset;
    }
    return std::make_unique<VpModel>(device, vertices);
}
void MainApp::loadGameObjects() {
    std::shared_ptr<VpModel> vpModel = createCubeModel(vpDevice, { 0.f, 0.f, 0.f });

    auto cube = VpGameObject::createGameObject();
    cube.model = vpModel;
    cube.transform.translation = { .0f, .0f, 2.5f };
    cube.transform.scale = { .5f, .5f, .5f };
    gameObjects.push_back(std::move(cube));
}

} // namespace vp