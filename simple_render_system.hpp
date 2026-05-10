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
class SimpleRenderSystem {
public:
    SimpleRenderSystem(VpDevice &device, VkRenderPass renderPass);
    ~SimpleRenderSystem();

    SimpleRenderSystem(const SimpleRenderSystem&) = delete;
    SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;
    void renderGameObjects(FrameInfo& frameInfo,std::vector<VpGameObject> &gameObjects );

private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);
    VpDevice &vpDevice;
    std::unique_ptr<VpPipeline> vpPipeline;
    VkPipelineLayout pipelineLayout;
};
} // namespace vp