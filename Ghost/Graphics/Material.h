#pragma once

#include "Vulkan_Init.h"
#include "Texture.h"

namespace graphics
{
	struct MaterialManager
	{
		std::vector<Texture> m_textures;
		std::vector<std::string> m_paths;
		DeviceInfo* m_deviceInfo = nullptr;
		VkFormat m_format{};
		size_t m_size = 0;

		void InitMaterials(DeviceInfo* deviceInfo);
		void CreateImage();
		void UpdateMaterial(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout);
		void Destroy();
	};
}