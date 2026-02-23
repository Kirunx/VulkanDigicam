#include "vp_pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace vp {
VpPipeline::VpPipeline(const std::string& vertFilepath,const std::string& fragFilepath){
	createGraphicsPipeline(vertFilepath,fragFilepath);
}

std::vector<char> VpPipeline::readFile(const std::string& filepath) {
    std::fstream file{filepath, std::ios::ate | std::ios::binary | std::ios::in};

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filepath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());

    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}
void VpPipeline::createGraphicsPipeline(const std::string& vertFilepath,
                                        const std::string& fragFilepath) {
    auto vertCode = readFile(vertFilepath);
    auto fragCode = readFile(fragFilepath);

	std::cout << "Vertex shader code size: " << vertCode.size() << '\n';
	std::cout << "Fragment shader code size: " << fragCode.size() << '\n';
}
}  // namespace vp