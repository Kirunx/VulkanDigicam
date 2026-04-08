#include "vp_window.hpp"
#include <stdexcept>
namespace vp {
VpWindow::VpWindow(int w, int h, std::string name)
    : width { w }
    , height { h }
    , windowName { name } {
    initWindow();
}
VpWindow::~VpWindow() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void VpWindow::initWindow() {
    glfwInit();
    // glfwWindowHint(GLFW_PLATFORM,GLFW_PLATFORM_X11);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window,this);
    glfwSetFramebufferSizeCallback(window,framebufferWindowResizedCallback);
}
void VpWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
}

void VpWindow::framebufferWindowResizedCallback(GLFWwindow* window, int width, int height) {
    auto vpWindow = reinterpret_cast<VpWindow*>(glfwGetWindowUserPointer(window));
    vpWindow->framebufferResized = true;
    vpWindow->width = width;
    vpWindow->height = height;

}
} // namespace vp