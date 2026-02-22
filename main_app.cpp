#include "main_app.hpp"

namespace vp {
void MainApp::run() {
	while(!vpWindow.shouldClose()){
		glfwPollEvents();
	}
}
}  // namespace vp