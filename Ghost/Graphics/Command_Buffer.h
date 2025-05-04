#pragma once
#include "Tools.h"
#include "Vulkan_Init.h"
#include "Buffer.h"
#include "Model.h"
#include "backends/imgui_impl_vulkan.h"
#include "SkyBox.h"

namespace graphics
{
	struct UboInfo
	{
		
		glm::mat4 proj = glm::mat4(0.0f);
		glm::mat4 view = glm::mat4(0.0f);
		glm::mat4 depthBiasMVP = glm::mat4(1.0f);
		glm::vec4 lightPos = glm::vec4(27.5474, -52.6093, 200.237, 1.0f);
		glm::vec3 viewPos = glm::vec3(0.0f);
		float zNear;
		float zFar;
	};

	struct UboInfoOffscreen
	{
		glm::mat4 depthMVP;
	};

	struct CommandBuffer
	{
		DeviceInfo* m_deviceInfo = nullptr;
		OffscreenData* m_offScreenData = nullptr;
		skybox* m_skyBox = nullptr;

		Buffer UniformBuffer;
		Buffer offscreenUniformBuffer;

		void Init(DeviceInfo* deviceInfo, OffscreenData* offScreenData, skybox* skyBox);
		void shutDown();
		void Prepare_PipelineLayout();
		void Allocate_CommandBuffer();
		void CreatePrimitiveSynchronizationObject();
		void InitDescriptors();
		void CreateDescriptorSet();
		void CreatePipelineCache();
		void SavePipelineCache();
		void CreateBindlessDescriptor();
		void CreateImmutableSamplerDescriptorSet();
	};

	
}