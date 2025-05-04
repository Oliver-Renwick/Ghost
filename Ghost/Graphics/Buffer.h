#pragma once
#include <vulkan/vulkan.h>
#include <assert.h>
#include <iostream>
#include "Tools.h"

namespace graphics
{
	struct Buffer
	{
		VkDevice m_device;
		VkPhysicalDevice m_PhysicalDevice;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDeviceSize BufferSize = 0;
		VkMemoryPropertyFlags memProps;
		void* mapped = nullptr;
		
		void Allocate(VkPhysicalDevice m_PhysicalDevice, VkDevice device, void* data, int size, VkBufferUsageFlags usageFlag, VkMemoryPropertyFlags memProps);
		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void Unmap();
		VkResult bind(VkDeviceSize offset = 0);
		void copyTo(void* data, VkDeviceSize size);
		void Destroy();
	};
}
