#pragma once
#include "Graphics/Vulkan_Init.h"
#include "imgui.h"

namespace ui
{
	struct UI
	{
		graphics::DeviceInfo* m_deviceInfo{};
		bool showDemoWindow = true;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		void Init_ImguiUI();
		void ImGui_Render(VkCommandBuffer cmdBuffer);
		void Cleanup();

	};
}