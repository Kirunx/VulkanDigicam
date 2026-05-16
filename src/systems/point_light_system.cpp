#include "point_light_system.hpp"

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

struct PointLightPushConstants {
    glm::vec4 position { };
    glm::vec4 color { };
    float radius;
};

PointLightSystem::PointLightSystem(VpDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : vpDevice { device } {
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() {
    vkDestroyPipelineLayout(vpDevice.device(), pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
    VkPushConstantRange pushConstantRange { };
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PointLightPushConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts { globalSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo { };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(vpDevice.device(), &pipelineLayoutInfo, nullptr,
            &pipelineLayout)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout ");
    }
}

void PointLightSystem::createPipeline(VkRenderPass renderPass) {
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig { };
    VpPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.bingingDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    vpPipeline = std::make_unique<VpPipeline>(
        vpDevice,
        "shaders/point_light.vert.spv",
        "shaders/point_light.frag.spv",
        pipelineConfig);
}

void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
    int lightIndex = 0;
    auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime, { 0.f, -1.f, 0.f });
    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.pointLight == nullptr)  continue;

        assert (lightIndex < MAX_LIGHTS && "Point lights exceeded maxium light capacity");

        obj.transform.translation = glm::vec3(rotateLight *  glm::vec4(obj.transform.translation, 1.0));
        
        ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.0);
        ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
        lightIndex += 1;
    }
    ubo.numLights = lightIndex;
}

void PointLightSystem::render(FrameInfo& frameInfo) {
    vpPipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0, 
        nullptr);

    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.pointLight == nullptr)
            continue;
        PointLightPushConstants push { };
        push.position = glm::vec4(obj.transform.translation, 1.0f);
        push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
        push.radius = obj.transform.scale.x;
        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PointLightPushConstants),
            &push
        );
        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }
}

} // namespace vp