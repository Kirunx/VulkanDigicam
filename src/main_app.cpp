#include "main_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"
#include "vp_buffer.hpp"
#include "vp_camera.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace vp {

MainApp::MainApp() {
    globalPool = VpDescriptorPool::Builder(vpDevice)
                     .setMaxSets(VpSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VpSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .build();
    loadGameObjects();
}

MainApp::~MainApp() { }

void MainApp::run() {
    std::vector<std::unique_ptr<VpBuffer>> uboBuffers(VpSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<VpBuffer>(
            vpDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }

    auto globalSetLayout = VpDescriptorSetLayout::Builder(vpDevice)
                               .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                               .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(VpSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        VpDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    SimpleRenderSystem simpleRenderSystem {
        vpDevice,
        vpRenderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout()
    };
    PointLightSystem pointLightSystem {
        vpDevice,
        vpRenderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout()
    };
    VpCamera camera { };

    auto viewerObject = VpGameObject::createGameObject();
    viewerObject.transform.translation.z = -2.5f;
    KeyboardMovementController cameraController { };

    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!vpWindow.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(vpWindow.getGlFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = vpRenderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (auto commandBuffer = vpRenderer.beginFrame()) {
            int frameIndex = vpRenderer.getFrameIndex();
            FrameInfo frameInfo {
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                gameObjects
            };

            // update
            GlobalUbo ubo { };
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            pointLightSystem.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // render
            vpRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);
            vpRenderer.endSwapChainRenderPass(commandBuffer);
            vpRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(vpDevice.device());
}

void MainApp::loadGameObjects() {
    std::shared_ptr<VpModel> vpModel = VpModel::createModelFromFile(vpDevice, "models/flat_vase.obj");
    auto flatVase = VpGameObject::createGameObject();
    flatVase.model = vpModel;
    flatVase.transform.translation = { -.5f, .5f, 0.f };
    flatVase.transform.scale = { 3.f, 1.5f, 3.f };
    gameObjects.emplace(flatVase.getId(), std::move(flatVase));

    vpModel = VpModel::createModelFromFile(vpDevice, "models/smooth_vase.obj");
    auto smoothVase = VpGameObject::createGameObject();
    smoothVase.model = vpModel;
    smoothVase.transform.translation = { .5f, .5f, 0.f };
    smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
    gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

    vpModel = VpModel::createModelFromFile(vpDevice, "models/quad.obj");
    auto floor = VpGameObject::createGameObject();
    floor.model = vpModel;
    floor.transform.translation = { 0.f, .5f, 0.f };
    floor.transform.scale = { 3.f, 1.f, 3.f };
    gameObjects.emplace(floor.getId(), std::move(floor));

    std::vector<glm::vec3> lightColors {
        { 1.f, .1f, .1f },
        { .1f, .1f, 1.f },
        { .1f, 1.f, .1f },
        { 1.f, 1.f, .1f },
        { .1f, 1.f, 1.f },
        { 1.f, 1.f, 1.f } //
    };

    for (int i = 0; i < lightColors.size(); ++i) {

        auto pointLight = VpGameObject::makePointLight(0.6f);
        pointLight.color = lightColors[i];
        auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f, -1.f, 0.f });
        pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }
}

} // namespace vp
