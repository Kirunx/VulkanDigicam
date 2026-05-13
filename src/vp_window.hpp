#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vp {
class VpWindow {
   public:
    VpWindow(int w, int h, std::string name);
    ~VpWindow();

    VpWindow(const VpWindow&) = delete;
    VpWindow& operator=(const VpWindow&) = delete;

    bool shouldClose() { return glfwWindowShouldClose(window); }
    VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
    bool wasWindowResized() {return framebufferResized;}
    void resetWindowResizedFlag() {framebufferResized = false;}

    GLFWwindow *getGlFWwindow() const {return window;} 

    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

   private:
   static void framebufferWindowResizedCallback(GLFWwindow *window, int width, int height);
    void initWindow();

    int width;
    int height;
    bool framebufferResized = false;

    std::string windowName;

    GLFWwindow* window;
};
}  // namespace vp
