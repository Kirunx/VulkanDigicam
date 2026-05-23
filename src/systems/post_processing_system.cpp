#include "post_processing_system.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>
#include <stdexcept>

namespace vp {

PostProcessRenderSystem::PostProcessRenderSystem(VpDevice& device, VkRenderPass renderPass)
    : vpDevice { device } {
    params = { glm::vec2(0, 0), 1.1f, 1.2f, 1.0f, 1.0f, 0.8f };
    createPipelineLayout();
    createPipeline(renderPass);
    loadNoiseTexture();
}

PostProcessRenderSystem::~PostProcessRenderSystem() {
    vkDestroyPipelineLayout(vpDevice.device(), pipelineLayout, nullptr);
}

void PostProcessRenderSystem::createPipelineLayout() {
    VkPushConstantRange pushRange { };
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(PostProcessPushConstants);

    descriptorSetLayout = VpDescriptorSetLayout::Builder(vpDevice)
                              .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                              .build();

    VkDescriptorSetLayout layoutHandle = descriptorSetLayout->getDescriptorSetLayout();
    VkPipelineLayoutCreateInfo info { };
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = 1;
    info.pSetLayouts = &layoutHandle;
    info.pushConstantRangeCount = 1;
    info.pPushConstantRanges = &pushRange;
    if (vkCreatePipelineLayout(vpDevice.device(), &info, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create post-process pipeline layout");
}

void PostProcessRenderSystem::createPipeline(VkRenderPass renderPass) {
    PipelineConfigInfo config { };
    VpPipeline::defaultPipelineConfigInfo(config);
    config.renderPass = renderPass;
    config.pipelineLayout = pipelineLayout;
    config.depthStencilInfo.depthTestEnable = VK_FALSE;
    config.depthStencilInfo.depthWriteEnable = VK_FALSE;
    config.colorBlendAttachment.blendEnable = VK_FALSE;

    vpPipeline = std::make_unique<VpPipeline>(
        vpDevice,
        "shaders/post_process.vert.spv",
        "shaders/post_process.frag.spv",
        config);
}

void PostProcessRenderSystem::loadNoiseTexture() {
    noiseTexture = VpTexture::createTextureFromFile(vpDevice, "../textures/noise.jpg");
}

void PostProcessRenderSystem::render(FrameInfo& frameInfo, VkImageView sceneView) {
    vpPipeline->bind(frameInfo.commandBuffer);

    // Update resolution
    auto extent = frameInfo.swapchainExtent;
    params.resolution = glm::vec2(static_cast<float>(extent.width), static_cast<float>(extent.height));

    // Scene texture descriptor (manually construct)
    VkDescriptorImageInfo sceneInfo { };
    sceneInfo.imageView = sceneView;
    sceneInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    sceneInfo.sampler = noiseTexture->sampler(); // reuse same sampler settings

    auto noiseInfo = noiseTexture->getImageInfo(); // already has sampler+view+layout

    VpDescriptorWriter writer(*descriptorSetLayout, frameInfo.frameDescriptorPool);
    writer.writeImage(0, &sceneInfo);
    writer.writeImage(1, &noiseInfo);
    VkDescriptorSet postSet;
    writer.build(postSet);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &postSet, 0, nullptr);

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(PostProcessPushConstants), &params);

    vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0); // full-screen triangle
}

} // namespace vp