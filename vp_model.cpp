#include "vp_model.hpp"

#include <cassert>
#include <cstring>
namespace vp {
VpModel::VpModel(VpDevice& device, const std::vector<Vertex>& vertices)
    : vpDevice { device } {
    createVertexBuffer(vertices);
}
VpModel::~VpModel() {
    vkDestroyBuffer(vpDevice.device(), vertexBuffer, nullptr);
    vkFreeMemory(vpDevice.device(), vertexBufferMemory, nullptr);
}
void VpModel::createVertexBuffer(const std::vector<Vertex>& vertices) {
    vertextCount = static_cast<uint32_t>(vertices.size());
    assert(vertextCount >= 3 && "Vertex count should be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertextCount;
    vpDevice.createBuffer(bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer,
        vertexBufferMemory);
    void* data;
    vkMapMemory(vpDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(vpDevice.device(), vertexBufferMemory);
}

void VpModel::draw(VkCommandBuffer commandBuffer) {
    vkCmdDraw(commandBuffer, vertextCount, 1, 0, 0);
}

void VpModel::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

std::vector<VkVertexInputBindingDescription> VpModel::Vertex::getBindingDesctriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VpModel::Vertex::getAttributeDesctriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = 0;
    return attributeDescriptions;
}
} // namespace vp