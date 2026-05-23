#pragma once
// std
#include <cassert>
#include <memory>
#include <vector>
#include <stdexcept>

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
    float getAspectRatio() const {
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

    void beginSceneRenderPass(VkCommandBuffer commandBuffer);
    void endSceneRenderPass(VkCommandBuffer commandBuffer);
    void recreateSceneRenderTarget();

    VkRenderPass getSceneRenderPass() const { return sceneTarget.renderPass; }
    VkFramebuffer getSceneFramebuffer() const { return sceneTarget.framebuffer; }
    VkImageView getSceneColorView() const { return sceneTarget.colorImageView; }

    VkExtent2D getSwapChainExtent() const { return vpSwapChain->getSwapChainExtent(); }

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

    struct SceneTarget {
        VkImage colorImage = VK_NULL_HANDLE;
        VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
        VkImageView colorImageView = VK_NULL_HANDLE;

        VkImage depthImage = VK_NULL_HANDLE;
        VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
        VkImageView depthImageView = VK_NULL_HANDLE;

        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
    };
    SceneTarget sceneTarget;

    void createSceneRenderPass();
    void createSceneRenderTarget();
    void cleanupSceneRenderTarget();
    void createImage(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage,
        VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo { };
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.extent = { extent.width, extent.height, 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(vpDevice.device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
            throw std::runtime_error("failed to create image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(vpDevice.device(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo { };
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vpDevice.findMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(vpDevice.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate image memory!");

        vkBindImageMemory(vpDevice.device(), image, imageMemory, 0);
    }
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo { };
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(vpDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            throw std::runtime_error("failed to create image view!");

        return imageView;
    }
};
} // namespace vp