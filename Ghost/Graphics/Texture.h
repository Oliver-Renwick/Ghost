#pragma once
#include<vulkan/vulkan.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <string>
#include <assert.h>
#include <iostream>
#include <vector>

#include "Tools.h"

namespace graphics
{
	struct Texture
	{
	public:
		VkDevice              m_device;
		VkPhysicalDevice      m_PhysicalDevice;
		VkQueue               m_queue;
		VkImage               m_image;
		VkImageView           m_imageView;
		VkSampler             m_sampler;
		VkImageLayout         m_imageLayout;
		VkDeviceMemory        m_deviceMemory;
		uint32_t              m_height, m_width;
		uint32_t              m_miplevels;
		uint32_t              m_layerCount;
		VkDescriptorImageInfo m_descriptorimageInfo;

		void updateDescriptor();
		void Destroy();

		ktxResult loadKtxFile(std::string filename, ktxTexture** target);
	};

	struct Texture2D : public Texture
	{
	public:
		void loadFromFile(
			std::string        filename,
			VkFormat           format,
			VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			bool               forceLiner = false);


		void FromBuffer(
			void* buffer,
			VkDeviceSize       bufferSize,
			VkFormat           format,
			uint32_t           texWidth,
			uint32_t           texHeight,
			VkFilter           filter = VK_FILTER_LINEAR,
			VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	};
}
