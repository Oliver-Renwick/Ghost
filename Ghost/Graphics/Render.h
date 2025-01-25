#pragma once
#include "Tools.h"
#include "Vulkan_Init.h"
#include "Model.h"
#include "UI/UI.h"

namespace graphics
{
	

	struct Renderer
	{
		DeviceInfo*	   m_deviceInfo	   = nullptr;
		OffscreenData* m_offScreenInfo = nullptr;
		uint32_t       currentFrame = 0;
		VkImage        depthImage;
		VkImageView    depthView;
		VkDeviceMemory DepthImageMemory;
		VkFormat       depthFormat;

		void   Init(DeviceInfo* deviceInfo, OffscreenData* offScreenData);
		void   Shutdown();
		void   CreateDepthResource();
		void   CreateDefaultrenderPass_FrameBuffer();
		void   PrepareOffscreenRenderPass();
		void   CreateOffScreenFrameBuffer();

		void   preparePipeline();
		void   RecordCommandBuffer(VkCommandBuffer cmdBuffer,VkPipelineLayout layout, uint32_t imageIndex, std::vector<RenderModel>& renderModels, ui::UI* gui);

		void   Re_CreateSwapchain();
		void   DrawFrame(std::vector<RenderModel>& renderModels, ui::UI* gui);
	};
}