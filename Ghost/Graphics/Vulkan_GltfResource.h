#pragma once
#include "Vulkan_Init.h"

namespace graphics
{
	struct GltfModel
	{
		DeviceInfo m_deviceInfo;

		bool load_Model(std::string& filename);
	};
}