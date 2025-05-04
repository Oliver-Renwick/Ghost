#pragma once
#include "Tools.h"
#include "Vulkan_Init.h"
#include "Material.h"
#include "Model.h"
#include "SkyBox.h"
#include "UI/UI.h"

namespace graphics
{
	

	struct Renderer
	{
		DeviceInfo*	     m_deviceInfo	   = nullptr;
		OffscreenData*   m_offScreenInfo   = nullptr;
		skybox*          m_skyBox		   = nullptr;
		MaterialManager* m_material		   = nullptr;
		uint32_t		 currentFrame	   = 0;

		VkImage        depthImage;
		VkImageView    depthView;
		VkDeviceMemory DepthImageMemory;
		VkFormat       depthFormat;

		VkImage		   colorImage;
		VkImageView	   colorImageView;
		VkDeviceMemory colorImageMemory;


		void   Init(DeviceInfo* deviceInfo, OffscreenData* offScreenData, skybox* skyBox, MaterialManager* material);
		void   Shutdown();
		void   CreateDepthResource();
		void   CreateColorResource();
		void   CreateDefaultrenderPass_FrameBuffer();
		void   PrepareOffscreenRenderPass();
		void   CreateOffScreenFrameBuffer();

		void   preparePipeline();
		void   RecordCommandBuffer(VkCommandBuffer cmdBuffer,VkPipelineLayout layout, uint32_t imageIndex, std::vector<RenderModel>& renderModels);

		void   Re_CreateSwapchain();
		void   DrawFrame(std::vector<RenderModel>& renderModels);
	};
}