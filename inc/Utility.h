#include <string>
#include <vulkan/vulkan.h>
struct AppDetails {
	std::string m_appName;
	std::string m_authorName;
	std::string m_dateOfCreation;
	std::string m_description;
};

VkResult g_createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void g_destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);