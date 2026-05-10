#pragma once

#include "vp_camera.hpp"

// lib
#include <vulkan/vulkan.h>

namespace vp {
struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    VpCamera &camera;
};

} // namespace vp
