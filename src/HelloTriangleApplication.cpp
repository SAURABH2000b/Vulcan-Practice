#ifdef _WIN32
	#define NOMINMAX
#endif
#include <map>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>

#include <cstring>
#include "HelloTriangleApplication.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> g_validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> g_deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
	m_createSurface();
	m_pickPhysicalDevice();
	m_createLogicalDevice();
	m_createSwapChain();
	m_createImageViews();
	m_createRenderPass();
	m_createGraphicsPipeline();

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
	std::cout << "Available Vukan Instance Extensions:\n";
	for (const auto& extension : extensionsProperties) {
		std::cout << "----> " << extension.extensionName << '\n';
	}
	std::cout << '\n';
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

	if (g_createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("Failed to setup debug messenger!");
	}

}

void HelloTriangleApplication::m_createSurface()
{
	if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface)!=VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
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

#ifdef _DEBUG
	std::cout << "Available Gpu(s)/CPU:\n";
#endif

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
	std::cout << '\n';
#endif
	
}

int HelloTriangleApplication::m_rateDeviceSuitability(VkPhysicalDevice device)
{

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

#ifdef _DEBUG
	std::cout << "----> " << deviceProperties.deviceName << '\n';
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

	// Prefer a physical device that is having a single queue family supporting all the command types (for better performance)
	if (indices.m_graphicsFamily.value() == indices.m_presentFamily.value()) {
		score += 100;
	}
	else {
		score += 50;
	}

	// This application is an interactive application demanding certain device extensions
#ifdef _DEBUG
	std::cout << "----> " << "Device extensions supperted by " << deviceProperties.deviceName << " are:" << '\n';
#endif
	if (!m_checkDeviceExtensionSupport(device)) {
		return 0;
	}

	// The physical device should have following minimum swapchain support
	bool swapChainAdequate = false;
	SwapChainSupportDetails swapChainSupport = m_querySwapChainSupport(device);
	swapChainAdequate = !swapChainSupport.m_formats.empty() && !swapChainSupport.m_presentModes.empty();
	if (!swapChainAdequate) {
		return 0;
	}

	return score;
}

bool HelloTriangleApplication::m_checkDeviceExtensionSupport(VkPhysicalDevice device)
{

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(g_deviceExtensions.begin(), g_deviceExtensions.end());

#ifdef _DEBUG
	for (const auto& extension : availableExtensions) {
		std::cout << "--------> " << extension.extensionName << '\n';
	}
	std::cout << '\n';
#endif

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();

}

void HelloTriangleApplication::m_createLogicalDevice()
{

	QueueFamilyIndices indices = m_findQueueFamilies(m_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.m_graphicsFamily.value(),
		indices.m_presentFamily.value()
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	// Following fields are ignored by latest Vulkan implementations. These are specified anyways for backward compatibility.
	createInfo.enabledExtensionCount = static_cast<uint32_t>(g_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = g_deviceExtensions.data();

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
	vkGetDeviceQueue(m_device, indices.m_presentFamily.value(), 0, &m_presentQueue);

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

		// Drawing/Graphics support:
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.m_graphicsFamily = i;
		}

		// Presentation support:
		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentationSupport);
		if (presentationSupport) {
			indices.m_presentFamily = i;
		}

		// Check if all required features/command types are fullfilled by the queue families:
		if (indices.m_isComplete()) {
			break;
		}

		i++;
	}

	return indices;

}

SwapChainSupportDetails HelloTriangleApplication::m_querySwapChainSupport(VkPhysicalDevice device)
{

	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.m_capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.m_formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.m_formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.m_presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.m_presentModes.data());
	}

	return details;

}

VkSurfaceFormatKHR HelloTriangleApplication::m_chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::m_chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{

	// As our application is targeted for desktop or laptop, energy usage is not big concern.
	// So we can go for a less energy efficient but having better responsiveness mode VK_PRESENT_MODE_MAILBOX_KHR (triple buffering)
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	// Only VK_PRESENT_MODE_FIFO_KHR is guaranteed to be availabe
	return VK_PRESENT_MODE_FIFO_KHR; // Appropriate for energy critical platforms like mobiles and laptops

}

VkExtent2D HelloTriangleApplication::m_chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) { // Vulkan is able to find the correct swap chain image resolution for the GLFW window we used to create surface.
		return capabilities.currentExtent;
	}
	else { // Vulkan is NOT able to find the correct swap chain image resolution for the GLFW window we used to create surface.
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actualExtent.width = std::clamp(actualExtent.width, 
			capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, 
			capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

void HelloTriangleApplication::m_createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = m_querySwapChainSupport(m_physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = m_chooseSwapSurfaceFormat(swapChainSupport.m_formats);
	VkPresentModeKHR presentMode = m_chooseSwapPresentMode(swapChainSupport.m_presentModes);
	VkExtent2D extent = m_chooseSwapExtent(swapChainSupport.m_capabilities);

	uint32_t imageCount = swapChainSupport.m_capabilities.minImageCount + 1; // Recommended for better performance
	if (swapChainSupport.m_capabilities.maxImageCount > 0 && 
		imageCount > swapChainSupport.m_capabilities.maxImageCount) {
		imageCount = swapChainSupport.m_capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface; // We can have multiple surfaces
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // Use more than one for VR platforms
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = m_findQueueFamilies(m_physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.m_graphicsFamily.value(),indices.m_presentFamily.value() };
	if (indices.m_graphicsFamily != indices.m_presentFamily) { // Multiple queues present
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Used for simplicity
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else { // Single queue present
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Offers best performance
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.m_capabilities.currentTransform; // No transformation to be applied to output image
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Ignore alpha value of the image channel
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE; // Needed in case of swap chain recreation when window size changes

	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swap chain!");
	}

	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;

	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
}

void HelloTriangleApplication::m_createImageViews()
{
	m_swapChainImageViews.resize(m_swapChainImages.size());
	for (size_t i = 0; i < m_swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain image viewes!");
		}
	}
}

void HelloTriangleApplication::m_createRenderPass()
{

	// Index:
	// 1. Attachment(s) creation
	// 2. Subpass(es) creation
	// 3. Render pass creation
	
	// Attachment(s) (output buffers) creation:
	VkAttachmentDescription colorAttachment{}; // Can have multiple attachments each for color or depth or stencil.
	colorAttachment.format = m_swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Applicable for color and depth buffers.
														  // We will be clearing the swapchain image to constant
														  // (black) color before drawing anything to it.
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Applicable for color and depth buffers.
															// We want the rendered content to be read by display driver(s),
															// to be shown on the screen(s), so we need to store it.
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Applicable for stencil buffers. 
																	 // We will not use stencil buffer, so don't care.
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Applicable for stencil buffers. 
																	   // We will not use stencil buffer, so don't care.
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Subpass(es) creation:
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Sets the initial layout of the attachment.

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // This subpass is a graphics related subpass. 
																 // In future Vulkan will be supporting compute subpass, 
																 // so we need to be explicit about what kind of subpass this is.
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// The driver will create an array of attachment descriptions.
	// Each subpass will hold the reference(s) to the attachment(s)
	// using its/their index/indices in this array.
	// The index of the attachment in this array is directly referenced from the fragment  shader
	// using the 'layout(location = 0) out vec4 outColor;' directive

	// Other kinds of attachments that a subpass can reference are:
	// 1. pInputAttachments: Attachments that can be read by the shaders as inputs using 'layout(location = 0) in vec4 inColor;' directive
	// 2. pResolveAttachments: Attachments used for multisampling color attachments.
	// 3. pDepthStencilAttachment: Attachment for depth and stencil data.
	// 4. pPreserveAttachments: Attachments which will be modified by current subpass and needs to be preserved for other subpass(es).

	// Render pass creation:
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void HelloTriangleApplication::m_createGraphicsPipeline()
{
	// Index:
	// 1. Programmable stage creation
	// 2. Vertex input state creation
	// 3. Input assembly state creation
	// 4. Viewport state creation (Dynamic or static)
	// 5. Rasterizer state creation
	// 6. Multisampling state creation
	// 7. Depth and Stencil state creation
	// 8. Color blending state creation
	// 9. Pipeline Layout creation
	// 10. Graphics Pipeline creation

	auto vertexShaderCode = s_readFile("compiled_shaders/hello_triangle_vert.spv");
	auto fragmentShaderCode = s_readFile("compiled_shaders/hello_triangle_frag.spv");

	VkShaderModule vertexShaderModule = m_createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = m_createShaderModule(fragmentShaderCode);

	// Stage creation for vertex shader:
	VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main"; // Entry point of the shader module, mostly 'main'
	// Its possible to combine multiple shaders (of same type/stage) into a single shader module
	// and differentiate them using different entry point names

	// Stage creation for fragment shader:
	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vertexShaderStageInfo.module = fragmentShaderModule;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

	// Vertex input state creation:
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	// Input assembly state creation:
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport definition (for static viewport state):
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapChainExtent.width;
	viewport.height = (float)m_swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor rectangle definition (for static scissor state):
	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = m_swapChainExtent;

	// Dynamic state declaration for viewport and scissor (for dynamic viewport and scissor states):
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	// We can make other useful states like polygon mode (filled or wireframe) etc. as dynamic too, but
	// that requires certain extensions.
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1; // for multiple viewports we need to have a gpu feature enabled in logical device creation section.
	viewportState.scissorCount = 1;
	//viewportState.pViewports = &viewport;
	//viewportState.pScissors = &scissor;
	// As we are opting for dynamic viewport, viewportState.pScissors and viewportState.pViewports can be specified at the time of drawing.

	// Rasterizer state creation:
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; // When true, it clamps the fragments beyond near and far planes
											// to near and far planes instead of discarding them. This is useful
											// for shadow mapping. To turn it on we need have a 
											// gpu feature enabled in logical device creation section.
	rasterizer.rasterizerDiscardEnable = VK_FALSE; // Used to disable output to frame buffer. Geometry from
												   // vertex processing never passes through the rasterizer stage.
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Using any other mode other than VK_POLYGON_MODE_FILL requires
												   // a gpu feature to be enabled in logical device creation section.
	rasterizer.lineWidth = 1.0f; // Thickness of lines in terms of number of fragments. Thickness other than 1.0f
	// requires a gpu feature 'wideLines' to be enabled in logical device creation section.
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE; // Useful for shadow mapping.
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	// Multisampling state creation:
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// Depth and Stencil state creation:

	// Color blending state creation:
	VkPipelineColorBlendAttachmentState colorBlendAttachment{}; // Create one for each framebuffer.
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp=VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// Pipeline layout creation:
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout!");
	}

	// Graphics Pipeline creation:
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0; // A graphics pipeline object can utilize only one subpass of the render pass.
							  // In case we need to utilize multiple subpasses of a render pass then we need
							  // to create new graphics pipeline objects utilizing each of them.
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Used when we are deriving the current graphics pipeline object
													  // from another graphics pipeline object.
													  // For this we need to specify VK_PIPELINE_CREATE_DERIVATIVE_BIT flag in flags field.
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}// We can create multiple graphics pipelines using vkCreateGraphicsPipelines() function in a single call, 
	 // by passing their count and pointer to an array of VkGraphicsPipelineCreateInfo struct objects.
	
	// Cleanup:
	vkDestroyShaderModule(m_device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(m_device, fragmentShaderModule, nullptr);



}

VkShaderModule HelloTriangleApplication::m_createShaderModule(const std::vector<char>& code)
{

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
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

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	for (auto imageView : m_swapChainImageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}
	
	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	vkDestroySwapchainKHR(m_device,m_swapChain,nullptr);
	vkDestroyDevice(m_device, nullptr);
	vkDestroyInstance(m_instance, nullptr);

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

std::vector<char> HelloTriangleApplication::s_readFile(const std::string& filename)
{

	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open the file: " + filename + "!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}
