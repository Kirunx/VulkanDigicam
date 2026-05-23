#pragma once
#include "vp_descriptors.hpp"
#include "vp_device.hpp"
#include "vp_frame_info.hpp"
#include "vp_pipeline.hpp"
#include "vp_texture.hpp"
#include <memory>

namespace vp {

struct PostProcessPushConstants {
    glm::vec2 resolution;   // swapchain width/height
    float ctr;              // contrast
    float brt;              // brightness
    float str;              // saturation
    float zoom;             // zoom factor
    float glowThreshold;    // placeholder
};

class PostProcessRenderSystem {
public:
    PostProcessRenderSystem(VpDevice& device, VkRenderPass renderPass);
    ~PostProcessRenderSystem();

    PostProcessRenderSystem(const PostProcessRenderSystem&) = delete;
    PostProcessRenderSystem& operator=(const PostProcessRenderSystem&) = delete;

    // Render post-process quad: sceneView = color attachment from off-screen pass
    void render(FrameInfo& frameInfo, VkImageView sceneView);

    // Live parameter setters
    void setContrast(float v) { params.ctr = v; }
    void setBrightness(float v) { params.brt = v; }
    void setZoom(float v) { params.zoom = v; }

private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);
    void loadNoiseTexture();

    VpDevice& vpDevice;
    std::unique_ptr<VpPipeline> vpPipeline;
    VkPipelineLayout pipelineLayout;
    std::unique_ptr<VpDescriptorSetLayout> descriptorSetLayout;
    std::unique_ptr<VpTexture> noiseTexture;
    PostProcessPushConstants params;
};

} // namespace vp