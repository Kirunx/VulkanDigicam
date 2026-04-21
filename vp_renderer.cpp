#include "vp_renderer.hpp"

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

VpRenderer::VpRenderer(VpWindow& window, VpDevice& device)
    : vpWindow { window }
    , vpDevice { device } {
    recreateSwapChain();
    createCommandBuffers();
}

VpRenderer::~VpRenderer() {
    freeCommandBuffers();
}

void VpRenderer::recreateSwapChain() {
    auto extent = vpWindow.getExtent();
    while (extent.height == 0 || extent.width == 0) {
        extent = vpWindow.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(vpDevice.device());

    if (vpSwapChain == nullptr) {
        vpSwapChain = std::make_unique<VpSwapChain>(vpDevice, extent);
    } else {
        std::shared_ptr<VpSwapChain> oldSwapChain = std::move(vpSwapChain);
        vpSwapChain = std::make_unique<VpSwapChain>(vpDevice, extent, oldSwapChain);

        if (!oldSwapChain->compareSwapFormats(*vpSwapChain.get())) {
            throw std::runtime_error("Swap chain image (or depth) format has changed!");
        }

    }
    //
}

void VpRenderer::createCommandBuffers() {
    commandBuffers.resize(VpSwapChain::MAX_FRAMES_IN_FLIGHT);

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

void VpRenderer::freeCommandBuffers() {
    vkFreeCommandBuffers(vpDevice.device(), vpDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}

VkCommandBuffer VpRenderer::beginFrame() {
    assert(!isFrameStarted && "Can't call begin frame while already in progress!");

    auto result = vpSwapChain->acquireNextImage(&currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire next swapchain image ");
    }

    isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo { };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error(
            "failde to begin recording command buffer");
    }
    return commandBuffer;
}
void VpRenderer::endFrame() {
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress!");
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record comand buffer");
    }
    auto result = vpSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || VK_SUBOPTIMAL_KHR || vpWindow.wasWindowResized()) {
        vpWindow.resetWindowResizedFlag();
        recreateSwapChain();
    }
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swapchain image");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1 ) % VpSwapChain::MAX_FRAMES_IN_FLIGHT;
}
void VpRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a diffrent frame! ");
    VkRenderPassBeginInfo renderPassInfo { };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vpSwapChain->getRenderPass();
    renderPassInfo.framebuffer = vpSwapChain->getFrameBuffer(currentImageIndex);

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = vpSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues { };
    clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport { };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(vpSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(vpSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor { { 0, 0 }, vpSwapChain->getSwapChainExtent() };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
void VpRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a diffrent frame! ");
    vkCmdEndRenderPass(commandBuffer);
}
} // namespace vp