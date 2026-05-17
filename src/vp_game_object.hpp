#pragma once

#include "vp_model.hpp"
#include "vp_swap_chain.hpp"
#include "vp_texture.hpp"
// libs
#include <glm/gtc/matrix_transform.hpp>
// std
#include <memory>
#include <unordered_map>

namespace vp {

struct TransformComponent {
    glm::vec3 translation;
    glm::vec3 scale { 1.f, 1.f, 1.f };
    glm::vec3 rotation { };

    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 mat4();
    glm::mat3 normalMatrix();
};

struct PointLightComponent {
    float lightIntensity = 1.0f;
};

struct GameObjectBufferData {
    glm::mat4 modelMatrix { 1.f };
    glm::mat4 normalMatrix { 1.f };
};

class VpGameObjectManager;

class VpGameObject {
public:
    using id_t = unsigned int;
    using Map = std::unordered_map<id_t, VpGameObject>;

    static VpGameObject makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

    VpGameObject(VpGameObject&&) = default;
    VpGameObject(const VpGameObject&) = delete;
    VpGameObject& operator=(const VpGameObject&) = delete;
    VpGameObject& operator=(VpGameObject&&) = default;

    const id_t getId() {
        return id;
    }

    VkDescriptorBufferInfo getBufferInfo(int frameIndex);

    glm::vec3 color { };
    TransformComponent transform { };

    // Optional components
    std::shared_ptr<VpModel> model { };
    std::shared_ptr<VpTexture> diffuseMap = nullptr;
    std::unique_ptr<PointLightComponent> pointLight = nullptr;

private:
    VpGameObject(id_t objId, const VpGameObjectManager &manager);
    id_t id;

    const VpGameObjectManager& gameObjectManger;

    friend class VpGameObjectManager;
};

class VpGameObjectManager {
public:
    static constexpr int MAX_GAME_OBJECTS = 1000;

    VpGameObjectManager(VpDevice& device);
    VpGameObjectManager(const VpGameObjectManager&) = delete;
    VpGameObjectManager& operator=(const VpGameObjectManager&) = delete;
    VpGameObjectManager(VpGameObjectManager&&) = delete;
    VpGameObjectManager& operator=(VpGameObjectManager&&) = delete;

    VpGameObject& createGameObject() {
        assert(currentId < MAX_GAME_OBJECTS && "Max game object count exceeded!");
        auto gameObject = VpGameObject { currentId++, *this };
        auto gameObjectId = gameObject.getId();
        gameObject.diffuseMap = textureDefault;
        gameObjects.emplace(gameObjectId, std::move(gameObject));
        return gameObjects.at(gameObjectId);
    }

    VpGameObject& makePointLight(
        float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

    VkDescriptorBufferInfo getBufferInfoForGameObject(
        int frameIndex, VpGameObject::id_t gameObjectId) const {
        return uboBuffers[frameIndex]->descriptorInfoForIndex(gameObjectId);
    }

    void updateBuffer(int frameIndex);

    VpGameObject::Map gameObjects { };
    std::vector<std::unique_ptr<VpBuffer>> uboBuffers { VpSwapChain::MAX_FRAMES_IN_FLIGHT };

private:
    VpGameObject::id_t currentId = 0;
    std::shared_ptr<VpTexture> textureDefault;
};

} // vp