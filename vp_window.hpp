#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vp {
class VpWindow {
   public:
    VpWindow(int w, int h, std::string name);
    ~VpWindow();

	VpWindow(const VpWindow & ) = delete;
	VpWindow &operator=(const VpWindow &) = delete;

    bool shouldClose() {return glfwWindowShouldClose(window);}

   private:
    void initWindow();

    const int WIDTH;
    const int HEIGHT;

    std::string windowName;

    GLFWwindow* window;
};
}  // namespace vp
