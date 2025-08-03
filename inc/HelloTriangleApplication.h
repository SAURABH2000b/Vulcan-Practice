#define GLFW_INCLUDE_VULKAN
#include<glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <fstream>

#include "Utility.h"

struct QueueFamilyIndices {
	std::optional<uint32_t> m_graphicsFamily;
	std::optional<uint32_t> m_presentFamily;

	bool m_isComplete() {
		return m_graphicsFamily.has_value() && m_presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR m_capabilities;
	std::vector<VkSurfaceFormatKHR> m_formats;
	std::vector<VkPresentModeKHR> m_presentModes;
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
	std::vector<const char*> m_getRequiredExtensions();
	bool m_checkValidationLayerSupport();
	void m_setupDebugMessenger();
	void m_createSurface();
	void m_populateDebugMessengerCreateInfoStruct(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void m_pickPhysicalDevice();
	int m_rateDeviceSuitability(VkPhysicalDevice device);
	bool m_checkDeviceExtensionSupport(VkPhysicalDevice device);
	void m_createLogicalDevice();
	QueueFamilyIndices m_findQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails m_querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR m_chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR m_chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D m_chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void m_createSwapChain();
	void m_createImageViews();
	void m_createRenderPass();
	void m_createGraphicsPipeline();
	VkShaderModule m_createShaderModule(const std::vector<char>& code);
	void m_createFramebuffers();
	void m_createCommandPool();
	void m_createCommandBuffer();
	void m_recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t swapchainImageIndex);
	void m_drawFrame();
	void m_createSyncObjects();

	static VKAPI_ATTR VkBool32 VKAPI_CALL s_debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static std::vector<char> s_readFile(const std::string& filename);

	AppDetails m_appDetails{ "Hello Triangle", "Saurabh Bhurewar", "6th June, 2025", "Rendering triangle using Vulkan!" };
	GLFWwindow* m_window;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger; // Debug Callback
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_graphicsPipeline;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;
	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;
	VkFence m_inFlightFence;
};