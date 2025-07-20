#include <map>

#include <cstring>
#include "HelloTriangleApplication.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> g_validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
	const bool g_enableValidationLayers = false;
#else
	const bool g_enableValidationLayers = true;
#endif

void HelloTriangleApplication::m_run()
{

	m_initWindow();
	m_initVulkan();
	m_mainLoop();
	m_cleanup();

}

void HelloTriangleApplication::m_initWindow()
{

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

}

void HelloTriangleApplication::m_initVulkan()
{

	m_createInstance();
	m_setupDebugMessenger();
	m_pickPhysicalDevice();
	m_createLogicalDevice();

}

void HelloTriangleApplication::m_createInstance()
{

	if (g_enableValidationLayers && !m_checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layers are requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = m_getRequiredExtensions();

#if defined(__APPLE__)
	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (g_enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(g_validationLayers.size());
		createInfo.ppEnabledLayerNames = g_validationLayers.data();

		m_populateDebugMessengerCreateInfoStruct(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance!");

	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensionsProperties(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsProperties.data());

#ifdef _DEBUG
	std::cout << "Available Extensions:\n";
	for (const auto& extension : extensionsProperties) {
		std::cout << '\t' << extension.extensionName << '\n';
	}
#endif
}

bool HelloTriangleApplication::m_checkValidationLayerSupport()
{

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : g_validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
			return false;
	}
	return true;

}

void HelloTriangleApplication::m_setupDebugMessenger()
{

	if (!g_enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	m_populateDebugMessengerCreateInfoStruct(createInfo);

	if (g_createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("Failed to setup debug messenger!");
}

void HelloTriangleApplication::m_populateDebugMessengerCreateInfoStruct(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{

	createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = s_debugCallBack;
	createInfo.pUserData = &this->m_appDetails; // Optional

}

void HelloTriangleApplication::m_pickPhysicalDevice()
{
	// An ordered map to automatically sort candidates (GPUs or CPU) by increasing score. (score is the key).
	std::multimap<int, VkPhysicalDevice> candidates;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	std::cout << "Available Gpu(s)/CPU:\n";
	for (const auto& device : devices) {
		int score = m_rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Check if the best candidate is suitable at all 
	if (candidates.rbegin()->first > 0) {
		m_physicalDevice = candidates.rbegin()->second;
	}
	else {
		throw std::runtime_error("Failed to find a suitable GPU for this application!");
	}

#ifdef _DEBUG
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
	std::cout << "Selected physical device: " << deviceProperties.deviceName << "\n";
#endif
	
}

int HelloTriangleApplication::m_rateDeviceSuitability(VkPhysicalDevice device)
{

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

#ifdef _DEBUG
	std::cout << '\t' << deviceProperties.deviceName << '\n';
#endif

	int score = 0;

	// Usually discrete/dedicated GPU have the significant performance
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 100;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	// This application can't function without geometric shaders
	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	// This application can't function without queue families supporting certain command types
	QueueFamilyIndices indices = m_findQueueFamilies(device);
	if (!indices.m_isComplete()) {
		return 0;
	}

	return score;
}

void HelloTriangleApplication::m_createLogicalDevice()
{
	QueueFamilyIndices indices = m_findQueueFamilies(m_physicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.m_graphicsFamily.value();
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;

	// Following fields are ignored by latest Vulkan implementations. These are specified anyways for backward compatibility.
	createInfo.enabledExtensionCount = 0;

	if (g_enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(g_validationLayers.size());
		createInfo.ppEnabledLayerNames = g_validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}

	// Get handles to the queue(s).
	vkGetDeviceQueue(m_device, indices.m_graphicsFamily.value(), 0, &m_graphicsQueue);
}

QueueFamilyIndices HelloTriangleApplication::m_findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector <VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.m_graphicsFamily = i;
		}

		if (indices.m_isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

std::vector<const char*> HelloTriangleApplication::m_getRequiredExtensions()
{

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	if (glfwExtensionCount <= 0)
		throw std::runtime_error("Vulkan is not supported on your system!");

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	
	if (g_enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;

}

void HelloTriangleApplication::m_mainLoop()
{

	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
	}

}

void HelloTriangleApplication::m_cleanup()
{
	if (g_enableValidationLayers) {
		g_destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}

	vkDestroyInstance(m_instance, nullptr);
	vkDestroyDevice(m_device, nullptr);

	glfwDestroyWindow(m_window);
	glfwTerminate();

}


VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::s_debugCallBack(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		std::cerr << "(" + static_cast<AppDetails*>(pUserData)->m_appName + ") Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;

}