#pragma once

#include "vp_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace vp {
class VpModel {
   public:
	struct Vertex {
		glm::vec2 position;

		static std::vector<VkVertexInputBindingDescription> getBindingDesctriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDesctriptions();
	};




    VpModel(VpDevice &device,const std::vector<Vertex> &vertices);
    ~VpModel();

    VpModel(const VpModel&) = delete;
    VpModel& operator=(const VpModel&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

   private:
	void createVertexBuffer(const std::vector<Vertex> &vertices);

    VpDevice& vpDevice;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    uint32_t vertextCount;
};
}  // namespace vp