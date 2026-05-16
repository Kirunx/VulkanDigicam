#pragma once

// std
#include <memory>
#include <vector>

#include "vp_camera.hpp"
#include "vp_device.hpp"
#include "vp_game_object.hpp"
#include "vp_pipeline.hpp"
#include "vp_frame_info.hpp"


namespace vp {
class PointLightSystem {
public:
    PointLightSystem(VpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem&) = delete;
    PointLightSystem& operator=(const PointLightSystem&) = delete;

    void update(FrameInfo& frameInfo, GlobalUbo &ubo);
    void render(FrameInfo& frameInfo);

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);
    VpDevice &vpDevice;
    std::unique_ptr<VpPipeline> vpPipeline;
    VkPipelineLayout pipelineLayout;
};
} // namespace vp