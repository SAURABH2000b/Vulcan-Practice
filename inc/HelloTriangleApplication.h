#define GLFW_INCLUDE_VULKAN
#include<glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

#include "Utility.h"

struct QueueFamilyIndices {
	std::optional<uint32_t> m_graphicsFamily;

	bool m_isComplete() {
		return m_graphicsFamily.has_value();
	}
};

class HelloTriangleApplication {
public:
	void m_run();
private:
	void m_initVulkan();
	void m_mainLoop();
	void m_cleanup();
	void m_initWindow();
	void m_createInstance();
	bool m_checkValidationLayerSupport();
	void m_setupDebugMessenger();
	void m_populateDebugMessengerCreateInfoStruct(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void m_pickPhysicalDevice();
	int m_rateDeviceSuitability(VkPhysicalDevice device);
	void m_createLogicalDevice();
	QueueFamilyIndices m_findQueueFamilies(VkPhysicalDevice device);

	std::vector<const char*> m_getRequiredExtensions();

	static VKAPI_ATTR VkBool32 VKAPI_CALL s_debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	AppDetails m_appDetails{ "Hello Triangle", "Saurabh Bhurewar", "6th June, 2025", "Rendering triangle using Vulkan!" };
	GLFWwindow* m_window;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device;
	VkQueue m_graphicsQueue;

};