#pragma once
#include "Vulkan_Init.h"

#include <ktxvulkan.h>
#include <ktx.h>


namespace graphics
{
	struct Texture
	{
		VkImage			m_image;
		VkImageView		m_imageView;
		VkSampler		m_sampler;
		VkDeviceMemory  m_imageMemory;
		VkImageLayout	m_imageLayout;
		uint32_t		m_width, m_height;
		uint32_t		m_miplevels;

		DeviceInfo* m_deviceInfo = nullptr;

		void load_Texture(const std::string& filename, DeviceInfo* deviceInfo, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer, 
			VkSemaphore semaphore,VkFence fence ,const VkFormat& format,bool enableSampler, bool linearTiling = false);
		void destroy();
	};
}