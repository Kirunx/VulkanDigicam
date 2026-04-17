#include "main_app.hpp"

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

struct SimplePushConstantData {
    glm::mat2 transform{1.0f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

MainApp::MainApp() {
    loadGameObjects();
    createPipelineLayout();
    recreateSwapChain();
    createCommandBuffers();
}

MainApp::~MainApp() {
    vkDestroyPipelineLayout(vpDevice.device(), pipelineLayout, nullptr);
}
void MainApp::run() {
    std::cout << "maxPushConstantSize = " << vpDevice.properties.limits.maxPushConstantsSize << '\n';

    while (!vpWindow.shouldClose()) {
        glfwPollEvents();
        drawFrame();
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
    triangle.color = {.1f,.8f,.1f};
    triangle.transform2d.translation.x = .2f;
    triangle.transform2d.scale = {2.f,.5f};
    triangle.transform2d.rotation = .25f * glm::two_pi<float>();
    gameObjects.push_back(std::move(triangle));
}

void MainApp::createPipelineLayout() {
    VkPushConstantRange pushConstantRange { };
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo { };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(vpDevice.device(), &pipelineLayoutInfo, nullptr,
            &pipelineLayout)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout ");
    }
}

void MainApp::createPipeline() {
    assert(vpSwapChain != nullptr && "Cannot create pipelie before swap chain");
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig { };
    VpPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = vpSwapChain->getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    vpPipeline = std::make_unique<VpPipeline>(
        vpDevice, "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv", pipelineConfig);
}

void MainApp::recreateSwapChain() {
    auto extent = vpWindow.getExtent();
    while (extent.height == 0 || extent.width == 0) {
        extent = vpWindow.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(vpDevice.device());

    if (vpSwapChain == nullptr) {
        vpSwapChain = std::make_unique<VpSwapChain>(vpDevice, extent);
    } else {
        vpSwapChain = std::make_unique<VpSwapChain>(vpDevice, extent, std::move(vpSwapChain));
        if (vpSwapChain->imageCount() != commandBuffers.size()) {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }
    createPipeline();
}

void MainApp::createCommandBuffers() {
    commandBuffers.resize(vpSwapChain->imageCount());

    VkCommandBufferAllocateInfo allocInfo { };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vpDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(vpDevice.device(), &allocInfo,
            commandBuffers.data())
        != VK_SUCCESS) {
        throw std::runtime_error("failde to allocate command buffers");
    }
}

void MainApp::freeCommandBuffers() {
    vkFreeCommandBuffers(vpDevice.device(), vpDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}

void MainApp::recordCommandBuffer(int imageIndex) {
    VkCommandBufferBeginInfo beginInfo { };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error(
            "failde to begin recording command buffer");
    }

    VkRenderPassBeginInfo renderPassInfo { };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vpSwapChain->getRenderPass();
    renderPassInfo.framebuffer = vpSwapChain->getFrameBuffer(imageIndex);

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = vpSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues { };
    clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport { };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(vpSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(vpSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor { { 0, 0 }, vpSwapChain->getSwapChainExtent() };
    vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

    renderGameObjects(commandBuffers[imageIndex]);


    vkCmdEndRenderPass(commandBuffers[imageIndex]);
    if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record comand buffer");
    }
}

void MainApp::renderGameObjects(VkCommandBuffer commandBuffer){
    vpPipeline->bind(commandBuffer);

    for(auto& obj : gameObjects){

        obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());

        SimplePushConstantData push{};
        push.offset = obj.transform2d.translation;
        push.color = obj.color;
        push.transform = obj.transform2d.mat2();
        vkCmdPushConstants(commandBuffer,pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,0, sizeof(SimplePushConstantData),&push);
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}



void MainApp::drawFrame() {
    uint32_t imageIndex;
    auto result = vpSwapChain->acquireNextImage(&imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire next swapchain image ");
    }

    recordCommandBuffer(imageIndex);
    result = vpSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || VK_SUBOPTIMAL_KHR || vpWindow.wasWindowResized()) {
        vpWindow.resetWindowResizedFlag();
        recreateSwapChain();
        return;
    }
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swapchain image");
    }
}

} // namespace vp