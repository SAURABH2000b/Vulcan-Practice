#define GLFW_INCLUDE_VULKAN
#include<glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

class HelloTriangleApplication {
public:
	void m_run();
private:
	void m_initVulkan();
	void m_mainLoop();
	void m_cleanup();
	void m_initWindow();
	void m_createInstance();

	GLFWwindow* m_window;
	VkInstance m_instance;
};