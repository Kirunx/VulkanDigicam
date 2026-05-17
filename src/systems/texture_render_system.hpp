#pragma once

#include "vp_camera.hpp"
#include "vp_descriptors.hpp"
#include "vp_device.hpp"
#include "vp_frame_info.hpp"
#include "vp_game_object.hpp"
#include "vp_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace vp {
class TextureRenderSystem {
 public:
  TextureRenderSystem(
      VpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~TextureRenderSystem();

  TextureRenderSystem(const TextureRenderSystem &) = delete;
  TextureRenderSystem &operator=(const TextureRenderSystem &) = delete;

  void renderGameObjects(FrameInfo &frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  VpDevice &vpDevice;

  std::unique_ptr<VpPipeline> vpPipeline;
  VkPipelineLayout pipelineLayout;

  std::unique_ptr<VpDescriptorSetLayout> renderSystemLayout;
};
}  // namespace vp