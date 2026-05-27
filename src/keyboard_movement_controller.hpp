#pragma once
#include "systems/post_processing_system.hpp"
#include "vp_game_object.hpp"
#include "vp_window.hpp"

namespace vp {
class KeyboardMovementController {
public:
    struct KeyMappings {
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveForward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_E;
        int moveDown = GLFW_KEY_Q;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
        int zoomIn = GLFW_KEY_EQUAL;
        int zoomOut = GLFW_KEY_MINUS;
    };

    void moveInPlaneXZ(GLFWwindow* window, float dt, VpGameObject &gameObject);
    void zoom(GLFWwindow* window, PostProcessRenderSystem &postProc);
    KeyMappings keys {};
    float moveSpeed{3.f};
    float lookSpeed{1.5f};
};
}