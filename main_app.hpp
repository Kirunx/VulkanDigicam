#pragma once

#include <memory>
#include <vector>

#include "vp_device.hpp"
#include "vp_game_object.hpp"
#include "vp_renderer.hpp"
#include "vp_window.hpp"

namespace vp {
class MainApp {
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    MainApp();
    ~MainApp();

    MainApp(const MainApp&) = delete;
    MainApp& operator=(const MainApp&) = delete;

    void run();

private:
    void loadGameObjects();

    VpWindow vpWindow { WIDTH, HEIGHT, "Hello Vulkan!" };
    VpDevice vpDevice { vpWindow };
    VpRenderer vpRenderer { vpWindow, vpDevice };
    std::vector<VpGameObject> gameObjects;
};
} // namespace vp