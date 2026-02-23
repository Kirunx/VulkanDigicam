#pragma once

#include "vp_window.hpp"
#include "vp_pipeline.hpp"

namespace vp {
class MainApp {
   public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

	void run();
   private:
    VpWindow vpWindow{WIDTH,HEIGHT,"Hello Vulkan!"};
	VpPipeline vpPipeline{"./shaders/simple_shader.vert.spv","./shaders/simple_shader.frag.spv"};
};
}  // namespace vp