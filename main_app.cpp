#include "main_app.hpp"

#include <array>
#include <stdexcept>
namespace vp {

MainApp::MainApp() {
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

MainApp::~MainApp() {
    vkDestroyPipelineLayout(vpDevice.device(), pipelineLayout, nullptr);
}
void MainApp::run() {
    while (!vpWindow.shouldClose()) {
        glfwPollEvents();
		drawFrame();
    }
	vkDeviceWaitIdle(vpDevice.device());
}

void MainApp::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(vpDevice.device(), &pipelineLayoutInfo, nullptr,
                               &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout ");
    }
}

void MainApp::createPipeline() {
    auto pipelineConfig = VpPipeline::defaultPipelineConfigInfo(
        vpSwapChain.width(), vpSwapChain.height());
    pipelineConfig.renderPass = vpSwapChain.getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    vpPipeline = std::make_unique<VpPipeline>(
        vpDevice, "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv", pipelineConfig);
}

void MainApp::createCommandBuffers() {
    commandBuffers.resize(vpSwapChain.imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vpDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(vpDevice.device(), &allocInfo,
                                 commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failde to allocate command buffers");
    }

    for (int i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error(
                "failde to begin recording command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vpSwapChain.getRenderPass();
        renderPassInfo.framebuffer = vpSwapChain.getFrameBuffer(i);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = vpSwapChain.getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount =
            static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vpPipeline->bind(commandBuffers[i]);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record comand buffer");
        }
    }
}
void MainApp::drawFrame() {
    uint32_t imageIndex;
    auto result = vpSwapChain.acquireNextImage(&imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire next swapchain image ");
    }
	
	result = vpSwapChain.submitCommandBuffers(&commandBuffers[imageIndex],&imageIndex);

	if(result != VK_SUCCESS){
		throw std::runtime_error("failed to present swapchain image");
	}
}

}  // namespace vp