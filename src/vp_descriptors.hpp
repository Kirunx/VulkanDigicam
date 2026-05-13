#pragma once

#include "vp_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace vp {

class VpDescriptorSetLayout {
public:
    class Builder {
    public:
        Builder(VpDevice& vpDevice)
            : vpDevice { vpDevice } { }

        Builder& addBinding(
            uint32_t binding,
            VkDescriptorType descriptorType,
            VkShaderStageFlags stageFlags,
            uint32_t count = 1);
        std::unique_ptr<VpDescriptorSetLayout> build() const;

    private:
        VpDevice& vpDevice;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings { };
    };

    VpDescriptorSetLayout(
        VpDevice& vpDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~VpDescriptorSetLayout();
    VpDescriptorSetLayout(const VpDescriptorSetLayout&) = delete;
    VpDescriptorSetLayout& operator=(const VpDescriptorSetLayout&) = delete;

    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
    VpDevice& vpDevice;
    VkDescriptorSetLayout descriptorSetLayout;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

    friend class VpDescriptorWriter;
};

class VpDescriptorPool {
public:
    class Builder {
    public:
        Builder(VpDevice& vpDevice)
            : vpDevice { vpDevice } { }

        Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
        Builder& setMaxSets(uint32_t count);
        std::unique_ptr<VpDescriptorPool> build() const;

    private:
        VpDevice& vpDevice;
        std::vector<VkDescriptorPoolSize> poolSizes { };
        uint32_t maxSets = 1000;
        VkDescriptorPoolCreateFlags poolFlags = 0;
    };

    VpDescriptorPool(
        VpDevice& vpDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes);
    ~VpDescriptorPool();
    VpDescriptorPool(const VpDescriptorPool&) = delete;
    VpDescriptorPool& operator=(const VpDescriptorPool&) = delete;

    bool allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

    void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

    void resetPool();

private:
    VpDevice& vpDevice;
    VkDescriptorPool descriptorPool;

    friend class VpDescriptorWriter;
};

class VpDescriptorWriter {
public:
    VpDescriptorWriter(VpDescriptorSetLayout& setLayout, VpDescriptorPool& pool);

    VpDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
    VpDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

    bool build(VkDescriptorSet& set);
    void overwrite(VkDescriptorSet& set);

private:
    VpDescriptorSetLayout& setLayout;
    VpDescriptorPool& pool;
    std::vector<VkWriteDescriptorSet> writes;
};

}