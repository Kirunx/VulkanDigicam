#include "main_app.hpp"

#include <stdexcept>
namespace vp {


MainApp::MainApp() {
	createPipelineLayout();
	createPipeline();
	createCommandBuffers();
}

MainApp::~MainApp() {
	vkDestroyPipelineLayout(vpDevice.device(),pipelineLayout,nullptr);
}
void MainApp::run() {
    while (!vpWindow.shouldClose()) {
        glfwPollEvents();
    }
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

    void MainApp::createCommandBuffers(){}
    void MainApp::drawFrame(){}

}  // namespace vp