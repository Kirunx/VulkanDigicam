#include "bloom_smear_system.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/constants.hpp>
#include <stdexcept>

namespace vp {

BloomSmearSystem::BloomSmearSystem(VpDevice& device, VkRenderPass renderPass)
    : vpDevice{device} {
    
    // Defaults: threshold 0.8, streak 10.0, intensity 1.5, direction 1.0 (downwards)
    params = { glm::vec2(0,0), 0.8f, 10.0f, 1.5f, 1.0f };

    // Create a basic linear sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    
    if (vkCreateSampler(vpDevice.device(), &samplerInfo, nullptr, &bloomSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create bloom sampler!");
    }

    createPipelineLayout();
    createPipeline(renderPass);
}

BloomSmearSystem::~BloomSmearSystem() {
    vkDestroyPipelineLayout(vpDevice.device(), pipelineLayout, nullptr);
    vkDestroySampler(vpDevice.device(), bloomSampler, nullptr);
}

void BloomSmearSystem::createPipelineLayout() {
    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(BloomSmearPushConstants);

    descriptorSetLayout = VpDescriptorSetLayout::Builder(vpDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    VkDescriptorSetLayout layoutHandle = descriptorSetLayout->getDescriptorSetLayout();
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = 1;
    info.pSetLayouts = &layoutHandle;
    info.pushConstantRangeCount = 1;
    info.pPushConstantRanges = &pushRange;
    
    if (vkCreatePipelineLayout(vpDevice.device(), &info, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create bloom smear pipeline layout");
}

void BloomSmearSystem::createPipeline(VkRenderPass renderPass) {
    PipelineConfigInfo config{};
    VpPipeline::defaultPipelineConfigInfo(config);
    config.renderPass = renderPass;          
    config.pipelineLayout = pipelineLayout;
    config.depthStencilInfo.depthTestEnable = VK_FALSE;
    config.depthStencilInfo.depthWriteEnable = VK_FALSE;
    config.colorBlendAttachment.blendEnable = VK_FALSE;

    vpPipeline = std::make_unique<VpPipeline>(
        vpDevice,
        "shaders/bloom_smear.vert.spv",
        "shaders/bloom_smear.frag.spv",
        config);
}

void BloomSmearSystem::render(FrameInfo& frameInfo, VkImageView sceneView) {
    vpPipeline->bind(frameInfo.commandBuffer);

    params.resolution = glm::vec2(
        static_cast<float>(frameInfo.swapchainExtent.width), 
        static_cast<float>(frameInfo.swapchainExtent.height));

    VkDescriptorImageInfo sceneInfo{};
    sceneInfo.imageView = sceneView;
    sceneInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    sceneInfo.sampler = bloomSampler;

    VpDescriptorWriter writer(*descriptorSetLayout, frameInfo.frameDescriptorPool);
    writer.writeImage(0, &sceneInfo);
    
    VkDescriptorSet bloomSet;
    writer.build(bloomSet);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &bloomSet, 0, nullptr);

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(BloomSmearPushConstants), &params);

    vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
}

} // namespace vp