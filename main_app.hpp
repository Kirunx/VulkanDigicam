#pragma once

#include "vp_window.hpp"

namespace vp {
class MainApp {
   public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

	void run();
   private:
    VpWindow vpWindow{WIDTH,HEIGHT,"Hello Vulkan!"};
};
}  // namespace vp