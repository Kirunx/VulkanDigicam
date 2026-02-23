#pragma once

#include <string>
#include <vector>

namespace vp {
class VpPipeline {
   private:
    static std::vector<char> readFile(const std::string& filepath );

	void createGraphicsPipeline(const std::string& vertFilepath,const std::string& fragFilepath);
   public:
    VpPipeline(const std::string& vertFilepath,const std::string& fragFilepath);
    
};

//VpPipeline::VpPipeline(/* args */) {}

//VpPipeline::~VpPipeline() {}

}  // namespace vp
