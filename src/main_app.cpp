#include "main_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "systems/point_light_system.hpp"
#include "systems/post_processing_system.hpp"
#include "systems/texture_render_system.hpp"
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
#include <iostream>
#include <stdexcept>

namespace vp {

MainApp::MainApp() {
    globalPool = VpDescriptorPool::Builder(vpDevice)
                     .setMaxSets(VpSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VpSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .build();

    // build frame descriptor pools
    framePools.resize(VpSwapChain::MAX_FRAMES_IN_FLIGHT);
    auto framePoolBuilder = VpDescriptorPool::Builder(vpDevice)
                                .setMaxSets(1000)
                                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                                .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
    for (int i = 0; i < framePools.size(); i++) {
        framePools[i] = framePoolBuilder.build();
    }

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

    std::cout << "Alignment: " << vpDevice.properties.limits.minUniformBufferOffsetAlignment << "\n";
    std::cout << "atom size: " << vpDevice.properties.limits.nonCoherentAtomSize << "\n";

    PointLightSystem pointLightSystem {
        vpDevice,
        vpRenderer.getSceneRenderPass(),
        globalSetLayout->getDescriptorSetLayout()
    };
    TextureRenderSystem textureRenderSystem {
        vpDevice,
        vpRenderer.getSceneRenderPass(),
        globalSetLayout->getDescriptorSetLayout()
    };
    PostProcessRenderSystem postProcessSystem {
        vpDevice,
        vpRenderer.getSwapChainRenderPass()
    };
    VpCamera camera { };

    auto& viewerObject = gameObjectManager.createGameObject();
    viewerObject.transform.translation.z = -2.5f;
    KeyboardMovementController cameraController { };

    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!vpWindow.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(vpWindow.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = vpRenderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (auto commandBuffer = vpRenderer.beginFrame()) {
            int frameIndex = vpRenderer.getFrameIndex();
            framePools[frameIndex]->resetPool();
            FrameInfo frameInfo {
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                *framePools[frameIndex],
                gameObjectManager.gameObjects,
                vpRenderer.getSwapChainExtent()
            };

            // update
            GlobalUbo ubo { };
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            pointLightSystem.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // final step of update is updating the game objects buffer data
            // The render functions MUST not change a game objects transform data
            gameObjectManager.updateBuffer(frameIndex);

            // render
            vpRenderer.beginSceneRenderPass(commandBuffer);
            textureRenderSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);
            vpRenderer.endSceneRenderPass(commandBuffer);

            // === PASS 2: Apply Post-Processing to Swapchain ===
            vpRenderer.beginSwapChainRenderPass(commandBuffer);
            postProcessSystem.render(frameInfo, vpRenderer.getSceneColorView());
            vpRenderer.endSwapChainRenderPass(commandBuffer);

            vpRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(vpDevice.device());
}

void MainApp::loadGameObjects() {
    std::shared_ptr<VpModel> vpModel = VpModel::createModelFromFile(vpDevice, "models/flat_vase.obj");
    auto& flatVase = gameObjectManager.createGameObject();
    flatVase.model = vpModel;
    flatVase.transform.translation = { -.5f, .5f, 0.f };
    flatVase.transform.scale = { 3.f, 1.5f, 3.f };

    vpModel = VpModel::createModelFromFile(vpDevice, "models/smooth_vase.obj");
    auto& smoothVase = gameObjectManager.createGameObject();
    smoothVase.model = vpModel;
    smoothVase.transform.translation = { .5f, .5f, 0.f };
    smoothVase.transform.scale = { 3.f, 1.5f, 3.f };

    vpModel = VpModel::createModelFromFile(vpDevice, "models/quad.obj");
    std::shared_ptr<VpTexture> marbleTexture = VpTexture::createTextureFromFile(vpDevice, "../textures/image.png");
    auto& floor = gameObjectManager.createGameObject();
    floor.model = vpModel;
    floor.diffuseMap = marbleTexture;
    floor.transform.translation = { 0.f, .5f, 0.f };
    floor.transform.scale = { 3.f, 1.f, 3.f };

    std::vector<glm::vec3> lightColors {
        { 1.f, .1f, .1f },
        { .1f, .1f, 1.f },
        { .1f, 1.f, .1f },
        { 1.f, 1.f, .1f },
        { .1f, 1.f, 1.f },
        { 1.f, 1.f, 1.f } //
    };

    for (int i = 0; i < lightColors.size(); i++) {
        auto& pointLight = gameObjectManager.makePointLight(0.6f);
        pointLight.color = lightColors[i];
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            (i * glm::two_pi<float>()) / lightColors.size(),
            { 0.f, -1.f, 0.f });
        pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
    }
}

} // namespace vp
