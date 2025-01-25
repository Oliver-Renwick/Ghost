#pragma once
#include "Graphics/Vulkan_Init.h"

namespace ui
{
	struct UI
	{
		graphics::DeviceInfo* m_deviceInfo{};
		bool showDemoWindow = true;

		void Init_ImguiUI();
		void ImGui_Render(VkCommandBuffer cmdBuffer);
		void Cleanup();

	};
}