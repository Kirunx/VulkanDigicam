#pragma once

#include "vp_model.hpp"
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

class VpGameObject {
public:
    using id_t = unsigned int;
    using Map = std::unordered_map<id_t,VpGameObject>;
    static VpGameObject createGameObject() {
        static id_t currentId = 0;
        return VpGameObject { currentId++ };
    }

    static VpGameObject makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

    VpGameObject(const VpGameObject&) = delete;
    VpGameObject& operator=(const VpGameObject&) = delete;
    VpGameObject(VpGameObject&&) = default;
    VpGameObject& operator=(VpGameObject&&) = default;

    const id_t getId() {
        return id;
    }
    glm::vec3 color { };    
    TransformComponent transform { };

    // Optional components
    std::shared_ptr<VpModel> model { };
    std::unique_ptr<PointLightComponent> pointLight = nullptr;

private:
    VpGameObject(id_t objId)
        : id { objId } { }
    id_t id;
};

} // vp