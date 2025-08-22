#define GLFW_INCLUDE_VULKAN
#include<glfw3.h>
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <fstream>
#include <chrono>

#include "Utility.h"

const int MAX_FRAMES_IN_FLIGHT = 2; // Recommended value is 2.

struct QueueFamilyIndices {
	std::optional<uint32_t> m_graphicsFamily;
	std::optional<uint32_t> m_presentFamily;
	std::optional<uint32_t> m_transferFamily;

	bool m_isComplete() {
		return m_graphicsFamily.has_value() && m_presentFamily.has_value() && m_transferFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR m_capabilities{};
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
	void m_createGraphicsPipelineWithNoVertexInput();
	void m_createGraphicsPipelineWithVertexInput();
	void m_createDescriptorSetLayout();
	void m_createGraphicsPipelineWith3DSetup();
	VkShaderModule m_createShaderModule(const std::vector<char>& code);
	void m_createFramebuffers();
	void m_createCommandPool();
	void m_createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkMemoryPropertyFlags properties,
				VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void m_createVertexBuffer();
	void m_createIndexBuffer();
	void m_createUniformBuffers();
	void m_createDescriptorPool();
	void m_createDescriptorSets();
	uint32_t m_findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void m_createCommandBuffers();
	void m_recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t swapchainImageIndex);
	void m_copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void m_updateUniformBuffer(uint32_t currentFrame);
	void m_drawFrame();
	void m_createSyncObjects();
	void m_recreateSwapChain();
	void m_swapChainCleanup();
	void m_renderPassCleanup();

	static VKAPI_ATTR VkBool32 VKAPI_CALL s_debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static std::vector<char> s_readFile(const std::string& filename);
	static void s_frameBufferResizeCallback(GLFWwindow* window, int width, int height);

	AppDetails m_appDetails{ "Hello Triangle", "Saurabh Bhurewar", "6th June, 2025", "Rendering triangle using Vulkan!" };
	GLFWwindow* m_window;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger; // Debug Callback
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkQueue m_transferQueue;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	VkRenderPass m_renderPass;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_graphicsPipeline;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	VkCommandPool m_commandPool;
	VkCommandPool m_commandPoolForTransferQueue;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBufferMemories;
	std::vector<void*> m_uniformBufferMaps;
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	bool m_framebufferResized = false;
	uint32_t m_current_Frame = 0;
};