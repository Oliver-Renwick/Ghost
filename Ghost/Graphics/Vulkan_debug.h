#pragma once
#include <vulkan/vulkan.h>
#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <assert.h>


namespace debug
{
	VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	// Load debug function pointers and set debug callback
	void setupDebugging(VkInstance instance);
	// Clear debug callback
	void freeDebugCallback(VkInstance instance);
	// Used to populate a VkDebugUtilsMessengerCreateInfoEXT with our example messenger function and desired flags
	void setupDebugingMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCI);
}
