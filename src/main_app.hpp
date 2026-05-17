#pragma once

#include "vp_descriptors.hpp"
#include "vp_device.hpp"
#include "vp_game_object.hpp"
#include "vp_renderer.hpp"
#include "vp_window.hpp"

// std
#include <memory>
#include <vector>

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

    VpWindow vpWindow { WIDTH, HEIGHT, "Vulkan Tutorial" };
    VpDevice vpDevice { vpWindow };
    VpRenderer vpRenderer { vpWindow, vpDevice };

    // note: order of declarations matters
    std::unique_ptr<VpDescriptorPool> globalPool { };
    std::vector<std::unique_ptr<VpDescriptorPool>> framePools;
    VpGameObjectManager gameObjectManager { vpDevice };
};
} // namespace vp
