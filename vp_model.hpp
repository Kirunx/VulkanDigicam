#pragma once

#include "vp_device.hpp"
#include "vp_buffer.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
// std
#include <memory>
#include <vector>

namespace vp {
class VpModel {
public:
    struct Vertex {
        glm::vec3 position { };
        glm::vec3 color { };
        glm::vec3 normal { };
        glm::vec2 uv { };

        static std::vector<VkVertexInputBindingDescription> getBindingDesctriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDesctriptions();

        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };

    struct Builder {
        std::vector<Vertex> vertices { };
        std::vector<uint32_t> indices { };

        void loadModel(const std::string& filepath);
    };

    VpModel(VpDevice& device, const VpModel::Builder& builder);
    ~VpModel();

    VpModel(const VpModel&) = delete;
    VpModel& operator=(const VpModel&) = delete;

    static std::unique_ptr<VpModel> createModelFromFile(VpDevice& device, const std::string& filepath);

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

private:
    void createVertexBuffers(const std::vector<Vertex>& vertices);
    void createIndexBuffers(const std::vector<uint32_t>& indices);

    VpDevice& vpDevice;

    std::unique_ptr<VpBuffer> vertexBuffer;
    uint32_t vertexCount;

    bool hasIndexBuffer = false;
    std::unique_ptr<VpBuffer> indexBuffer;
    uint32_t indexCount;
};
} // namespace vp