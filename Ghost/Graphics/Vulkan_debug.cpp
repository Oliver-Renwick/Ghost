#include "Vulkan_debug.h"

std::wstring stringToWString(const std::string& str) {
	// Allocate enough space for the wide string
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
	return wstr;
}

namespace debug
{
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
	VkDebugUtilsMessengerEXT debugUtilsMessenger;

	VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::string prefix;

		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			prefix = "ERROR : ";
		}

		std::stringstream debugMessage;

		debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "][" << pCallbackData->pMessageIdName << "]  " << pCallbackData->pMessage << std::endl << std::endl;

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			std::cout << debugMessage.str() << '\n\n';

			//fflush(stdout);
		}



		return VK_FALSE;
	}


	void setupDebugingMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCI)
	{
		debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugUtilsMessengerCI.pfnUserCallback = debugUtilsMessageCallback;
	}

	void setupDebugging(VkInstance instance)
	{
		vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
		setupDebugingMessengerCreateInfo(debugUtilsMessengerCI);
		VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCI, nullptr, &debugUtilsMessenger);
		assert(result == VK_SUCCESS);
	}

	void freeDebugCallback(VkInstance instance)
	{
		if (debugUtilsMessenger != VK_NULL_HANDLE)
		{
			vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
		}
	}
}