#pragma once

#include "Vulkan_Init.h"
#include "Buffer.h"
#include "Texture.h"
#include "Material.h"
#include "TaskScheduler.h"

namespace graphics
{

	struct UploadRequest
	{
		Texture texture;
		Buffer buffer;
	};

	struct FileLoadRequest
	{
		std::string path;

		bool texure = false;
		bool buffer = false;
	};

	struct AsynchronousLoader
	{
		void Init(DeviceInfo* deviceInfo, MaterialManager* materials);
		void Update();
		void Shutdown();

		DeviceInfo* m_deviceInfo = nullptr;
		std::vector<VkCommandBuffer> m_cmdBuffer;
		std::vector<VkCommandPool>	m_cmdPool;
		VkFence			m_transferFence;
		VkSemaphore		m_transfer_complete_semaphore;
		MaterialManager* m_materials;

		std::queue<FileLoadRequest> m_fileloadRequests;

		void request_texture_request(std::string filename);
	};
}