#include "vp_window.hpp"
#include <stdexcept>
namespace vp {
VpWindow::VpWindow(int w, int h, std::string name)
    : width{w}, height{h}, windowName{name} {
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window =
        glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}
void VpWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
	if(glfwCreateWindowSurface(instance,window,nullptr,surface) != VK_SUCCESS){
		throw std::runtime_error("failed to create window surface");
	}
}
}  // namespace vp