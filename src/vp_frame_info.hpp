#pragma once

#include "vp_camera.hpp"
#include "vp_game_object.hpp"
// lib
#include <vulkan/vulkan.h>

namespace vp {
struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    VpCamera &camera;
    VkDescriptorSet globalDescriptorSet;
    VpGameObject::Map &gameObjects;
};

} // namespace vp
