#include "main_app.hpp"
#include "keyboard_movement_controller.hpp"
#include "simple_render_system.hpp"
#include "vp_camera.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace vp {

struct GlobalUbo {
    glm::mat4 projectionView { 1.f };
    glm::vec3 lightDirection = glm::normalize(glm::vec3 { 1.f, -3.f, -1.f });
};

MainApp::MainApp() {
    loadGameObjects();
}

MainApp::~MainApp() { }
void MainApp::run() {
    std::vector<std::unique_ptr<VpBuffer>> uboBuffers(VpSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); ++i) {
        uboBuffers[i] = std::make_unique<VpBuffer>(
            vpDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }

    SimpleRenderSystem simpleRenderSystme { vpDevice, vpRenderer.getSwapChainRenderPass() };
    VpCamera camera { };

    camera.setViewTarget(glm::vec3 { -1.f, -2.f, -2.f }, glm::vec3 { 0.f, 0.f, 2.5f });

    auto viewerObject = VpGameObject::createGameObject();
    KeyboardMovementController cameraController { };

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!vpWindow.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(vpWindow.getGlWFwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = vpRenderer.getAspectRation();

        // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 15.f);
        if (auto commandBuffer = vpRenderer.beginFrame()) {
            int frameIndex = vpRenderer.getFrameIndex();
            FrameInfo frameInfo {
                frameIndex,
                frameTime,
                commandBuffer,
                camera
            };

            // update
            GlobalUbo ubo { };
            ubo.projectionView = camera.getProjection() * camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();
            // render
            vpRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystme.renderGameObjects(frameInfo, gameObjects);
            vpRenderer.endSwapChainRenderPass(commandBuffer);
            vpRenderer.endFrame();
        }
    }
    vkDeviceWaitIdle(vpDevice.device());
}

void MainApp::loadGameObjects() {
    std::shared_ptr<VpModel> vpModel = VpModel::createModelFromFile(vpDevice, "models/flat_vase.obj");

    auto gameObj = VpGameObject::createGameObject();
    gameObj.model = vpModel;
    gameObj.transform.translation = { .0f, .5f, 2.5f };
    gameObj.transform.scale = glm::vec3 { 1.f, 0.5f, 3.f };
    gameObjects.push_back(std::move(gameObj));
}

} // namespace vp