#pragma once

#include <string>
#include <vector>

#include "vp_device.hpp"

namespace vp {

struct PipelineConfigInfo {
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};
class VpPipeline {
   private:
    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(const std::string& vertFilepath,
                                const std::string& fragFilepath,
                                const PipelineConfigInfo& configInfo);

    void createShaderModule(const std::vector<char>& code,
                            VkShaderModule* shaderModule);
    VpDevice& vpDevice;
    VkPipeline graphicsPipeline;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

   public:
    VpPipeline(VpDevice& device, const std::string& vertFilepath,
               const std::string& fragFilepath,
               const PipelineConfigInfo& configInfo);
    ~VpPipeline();

    VpPipeline(const VpPipeline&) = delete;
    void operator=(const VpPipeline&) = delete;

    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width,
                                                        uint32_t height);
};



}  // namespace vp
