#include "vp_renderer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <iostream>


namespace vp {

VpRenderer::VpRenderer(VpWindow& window, VpDevice& device)
    : vpWindow { window }
    , vpDevice { device } {
    recreateSwapChain();
    createCommandBuffers();
}

VpRenderer::~VpRenderer() {
    freeCommandBuffers();
    cleanupSceneRenderTarget();
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
    cleanupSceneRenderTarget();
    createSceneRenderPass();
    createSceneRenderTarget();
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
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vpWindow.wasWindowResized()) {
        vpWindow.resetWindowResizedFlag();
        recreateSwapChain();
    }
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swapchain image");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % VpSwapChain::MAX_FRAMES_IN_FLIGHT;
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

void VpRenderer::createSceneRenderPass() {
    // Color attachment
    VkAttachmentDescription colorAttach { };
    colorAttach.format = vpSwapChain->getSwapChainImageFormat(); // ✓ correct getter
    colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Depth attachment
    VkAttachmentDescription depthAttach { };
    depthAttach.format = vpSwapChain->findDepthFormat(); // ✓ use our helper, not swapchain method
    depthAttach.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthRef { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpass { };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttach, depthAttach };
    VkRenderPassCreateInfo rpInfo { };
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    rpInfo.pAttachments = attachments.data();
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(vpDevice.device(), &rpInfo, nullptr,
            &sceneTarget.renderPass)
        != VK_SUCCESS) {
        throw std::runtime_error("Failed to create scene render pass");
    }
}

void VpRenderer::createSceneRenderTarget() {
    auto extent = vpSwapChain->getSwapChainExtent();

    // === COLOR IMAGE ===
    createImage(extent, vpSwapChain->getSwapChainImageFormat(),
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        sceneTarget.colorImage, sceneTarget.colorImageMemory);

    sceneTarget.colorImageView = createImageView(
        sceneTarget.colorImage,
        vpSwapChain->getSwapChainImageFormat(),
        VK_IMAGE_ASPECT_COLOR_BIT);

    // === DEPTH IMAGE ===
    VkFormat depthFormat = vpSwapChain->findDepthFormat();
    createImage(extent, depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        sceneTarget.depthImage, sceneTarget.depthImageMemory);

    sceneTarget.depthImageView = createImageView(
        sceneTarget.depthImage,
        depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT);

    // === FRAMEBUFFER ===
    std::array<VkImageView, 2> attachments = {
        sceneTarget.colorImageView,
        sceneTarget.depthImageView
    };
    VkFramebufferCreateInfo fbInfo { };
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = sceneTarget.renderPass;
    fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    fbInfo.pAttachments = attachments.data();
    fbInfo.width = extent.width;
    fbInfo.height = extent.height;
    fbInfo.layers = 1;

    if (vkCreateFramebuffer(vpDevice.device(), &fbInfo, nullptr,
            &sceneTarget.framebuffer)
        != VK_SUCCESS) {
        throw std::runtime_error("Failed to create scene framebuffer");
    }
}

void VpRenderer::beginSceneRenderPass(VkCommandBuffer commandBuffer) {
    VkRenderPassBeginInfo renderPassInfo { };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = sceneTarget.renderPass;
    renderPassInfo.framebuffer = sceneTarget.framebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = vpSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues { };
    clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 }; 
    renderPassInfo.clearValueCount = clearValues.size();
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

void VpRenderer::endSceneRenderPass(VkCommandBuffer commandBuffer) {
    vkCmdEndRenderPass(commandBuffer);
}

void VpRenderer::cleanupSceneRenderTarget() {
    // Color
    if (sceneTarget.colorImageView) vkDestroyImageView(vpDevice.device(), sceneTarget.colorImageView, nullptr);
    if (sceneTarget.colorImage) {
        vkDestroyImage(vpDevice.device(), sceneTarget.colorImage, nullptr);
        vkFreeMemory(vpDevice.device(), sceneTarget.colorImageMemory, nullptr);
    }
    
    // Depth
    if (sceneTarget.depthImageView) vkDestroyImageView(vpDevice.device(), sceneTarget.depthImageView, nullptr);
    if (sceneTarget.depthImage) {
        vkDestroyImage(vpDevice.device(), sceneTarget.depthImage, nullptr);
        vkFreeMemory(vpDevice.device(), sceneTarget.depthImageMemory, nullptr);
    }
    
    // Framebuffer & render pass
    if (sceneTarget.framebuffer) vkDestroyFramebuffer(vpDevice.device(), sceneTarget.framebuffer, nullptr);
    if (sceneTarget.renderPass) vkDestroyRenderPass(vpDevice.device(), sceneTarget.renderPass, nullptr);
    
    // Reset handles
    sceneTarget = SceneTarget{};
}
} // namespace vp