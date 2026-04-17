#pragma once

#include <memory>
#include <vector>

#include "vp_device.hpp"
#include "vp_game_object.hpp"
#include "vp_pipeline.hpp"
#include "vp_swap_chain.hpp"
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
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void freeCommandBuffers();
    void drawFrame();
    void recreateSwapChain();
    void recordCommandBuffer(int imageIndex);
    void renderGameObjects(VkCommandBuffer commandBuffer);

    VpWindow vpWindow { WIDTH, HEIGHT, "Hello Vulkan!" };
    VpDevice vpDevice { vpWindow };
    std::unique_ptr<VpSwapChain> vpSwapChain;
    std::unique_ptr<VpPipeline> vpPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VpGameObject> gameObjects;
};
} // namespace vp