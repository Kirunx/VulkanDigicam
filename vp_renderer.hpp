#pragma once
// std
#include <cassert>
#include <memory>
#include <vector>

#include "vp_device.hpp"
#include "vp_swap_chain.hpp"
#include "vp_window.hpp"

namespace vp {
class VpRenderer {
public:
    VpRenderer(VpWindow& window, VpDevice& device);
    ~VpRenderer();

    VpRenderer(const VpRenderer&) = delete;
    VpRenderer& operator=(const VpRenderer&) = delete;

    VkRenderPass getSwapChainRenderPass() const { return vpSwapChain->getRenderPass(); }
    float getAspectRation() const {
        return vpSwapChain->extentAspectRatio();
    }
    bool isFrameInProgress() const { return isFrameStarted; }

    VkCommandBuffer getCurrentCommandBuffer() const {
        assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
        return commandBuffers[currentFrameIndex];
    }

    int getFrameIndex() const {
        assert(isFrameStarted && "Cannot get frame index when frame is not in progress");
        return currentFrameIndex;
    }

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    VpWindow& vpWindow;
    VpDevice& vpDevice;
    std::unique_ptr<VpSwapChain> vpSwapChain;
    std::vector<VkCommandBuffer> commandBuffers;

    uint32_t currentImageIndex;
    int currentFrameIndex { 0 };
    bool isFrameStarted { false };
};
} // namespace vp