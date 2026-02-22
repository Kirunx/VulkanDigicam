#include "vp_window.hpp"

namespace vp {
	VpWindow::VpWindow(int w, int h, std::string name) : WIDTH{w}, HEIGHT{h}, windowName{name} {
		initWindow();
	}
	VpWindow::~VpWindow(){
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VpWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);

		window = glfwCreateWindow(WIDTH,HEIGHT,windowName.c_str(),nullptr,nullptr);
	}
}// namespace lp