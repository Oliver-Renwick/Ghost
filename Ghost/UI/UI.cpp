#include "UI.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "backends/imgui_impl_vulkan.h"

static void check_vk_result(VkResult err)
{
	if (err == VK_SUCCESS)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

static ImGui_ImplVulkanH_Window g_MainWindowData;


namespace ui
{
	void UI::Init_ImguiUI()
	{
		



		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.DisplaySize = ImVec2(float(m_deviceInfo->width), float(m_deviceInfo->height));
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		ImGui_ImplWin32_Init(m_deviceInfo->hInst);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = m_deviceInfo->Instance;
		init_info.PhysicalDevice = m_deviceInfo->PhysicalDevice;
		init_info.Device = m_deviceInfo->Device;
		init_info.QueueFamily = 0;
		init_info.Queue = m_deviceInfo->GraphicsQueue;
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = m_deviceInfo->DescriptorPool;
		init_info.RenderPass = m_deviceInfo->renderPass;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = 2;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&init_info);

	}

	

	void UI::ImGui_Render(VkCommandBuffer cmdBuffer)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(float(m_deviceInfo->width), float(m_deviceInfo->height));


		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		ImGui::ShowDemoWindow();
		ImGui::Render();
		ImDrawData* draw_Data = ImGui::GetDrawData();

		const bool is_minimized = (draw_Data->DisplaySize.x <= 0.0f || draw_Data->DisplaySize.y <= 0.0f);

		// Record dear imgui primitives into command buffer
		if (!is_minimized)
		{
			ImGui_ImplVulkan_RenderDrawData(draw_Data, cmdBuffer);

		}
	}

	void UI::Cleanup()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}