#include "main_app.hpp"
#include "simple_render_system.hpp"
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
   SimpleRenderSystem simpleRenderSystme{vpDevice, vpRenderer.getSwapChainRenderPass()};

    while (!vpWindow.shouldClose()) {
        glfwPollEvents();

        if (auto commandBuffer = vpRenderer.beginFrame()) {
            vpRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystme.renderGameObjects(commandBuffer,gameObjects );
            vpRenderer.endSwapChainRenderPass(commandBuffer);
            vpRenderer.endFrame();
        }
    }
    vkDeviceWaitIdle(vpDevice.device());
}

void MainApp::loadGameObjects() {
    std::vector<VpModel::Vertex> vertices {
        { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
        { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
    };

    auto vpModel = std::make_shared<VpModel>(vpDevice, vertices);

    auto triangle = VpGameObject::createGameObject();
    triangle.model = vpModel;
    triangle.color = { .1f, .8f, .1f };
    triangle.transform2d.translation.x = .2f;
    triangle.transform2d.scale = { 2.f, .5f };
    triangle.transform2d.rotation = .25f * glm::two_pi<float>();
    gameObjects.push_back(std::move(triangle));
}


} // namespace vp