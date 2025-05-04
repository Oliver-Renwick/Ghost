#pragma once

#include "Vulkan_Init.h"
#include "Model.h"
#include "Texture.h"

namespace graphics
{
	struct skybox
	{
		//data
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_mipLevels;

		VkImage m_image;
		VkImageView m_imageView;
		VkSampler m_sampler;

		VkImageLayout m_imageLayout;
		VkDeviceMemory m_imageMemory;

		DeviceInfo* m_deviceInfo = nullptr;

		struct
		{
			VkDescriptorImageInfo imageInfo;
			VkDescriptorSet descriptorSet;
		}descriptorInfo;

		VkPipeline m_skyboxPipeline = VK_NULL_HANDLE;

		//Cube 
		std::vector<vert> m_vertices;
		std::vector<uint32_t> m_indices;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		void createCubemap(std::string filename, VkFormat format ,DeviceInfo* deviceInfo );
		void preparePipeline();
		void UpdateDescriptor();
		void DrawSkybox(VkCommandBuffer cmdBuffer);
		void Destroy();
		void CreateCube();
	};
}