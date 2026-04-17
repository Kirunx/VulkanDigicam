#pragma once

#include "vp_model.hpp"

// std
#include <memory>

namespace vp {

struct Transform2dComponent {
    glm::vec2 translation;
    glm::vec2 scale { 1.f, 1.f };
    float rotation;

    glm::mat2 mat2() {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotMatrix{{c,s},{-s,c}};
        glm::mat2 scaleMat{{scale.x, .0f},{.0f,scale.y}};
        return rotMatrix * scaleMat;
    };
};

class VpGameObject {
public:
    using id_t = unsigned int;
    static VpGameObject createGameObject() {
        static id_t currentId = 0;
        return VpGameObject { currentId++ };
    }
    VpGameObject(const VpGameObject&) = delete;
    VpGameObject& operator=(const VpGameObject&) = delete;
    VpGameObject(VpGameObject&&) = default;
    VpGameObject& operator=(VpGameObject&&) = default;

    const id_t getId() {
        return id;
    }
    std::shared_ptr<VpModel> model { };
    glm::vec3 color { };
    Transform2dComponent transform2d { };

private:
    VpGameObject(id_t objId)
        : id { objId } { }
    id_t id;
};

} // vp