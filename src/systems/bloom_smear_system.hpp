#pragma once
#include "vp_descriptors.hpp"
#include "vp_device.hpp"
#include "vp_frame_info.hpp"
#include "vp_pipeline.hpp"
#include <memory>

namespace vp {

struct BloomSmearPushConstants {
    glm::vec2 resolution;
    float threshold;
    float streakLength;
    float intensity;
    float direction;
};

class BloomSmearSystem {
public:
    BloomSmearSystem(VpDevice& device, VkRenderPass renderPass);
    ~BloomSmearSystem();

    BloomSmearSystem(const BloomSmearSystem&) = delete;
    BloomSmearSystem& operator=(const BloomSmearSystem&) = delete;

    void render(FrameInfo& frameInfo, VkImageView sceneView);

    void setThreshold(float v) { params.threshold = v; }
    void setStreakLength(float v) { params.streakLength = v; }
    void setIntensity(float v) { params.intensity = v; }
    void setDirection(float v) { params.direction = v; }

private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);

    VpDevice& vpDevice;
    std::unique_ptr<VpPipeline> vpPipeline;
    VkPipelineLayout pipelineLayout;
    std::unique_ptr<VpDescriptorSetLayout> descriptorSetLayout;
    VkSampler bloomSampler;
    
    BloomSmearPushConstants params;
};

} // namespace vp