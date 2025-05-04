#include "Asynchronous_Loader.h"


namespace graphics
{
	void AsynchronousLoader::Init(DeviceInfo* deviceInfo, MaterialManager* materials)
	{
		m_deviceInfo = deviceInfo;
		m_materials = materials;

		size_t frameSize = static_cast<size_t>(m_deviceInfo->MAX_FRAME_IN_FLIGHT);

		m_cmdPool.resize(frameSize);
		m_cmdBuffer.resize(frameSize);

		// Command Pool and Command Buffer Generation
		for (int i = 0; i < m_deviceInfo->MAX_FRAME_IN_FLIGHT; i++)
		{
			VkCommandPoolCreateInfo cmdPoolInfo{};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = m_deviceInfo->graphicsQueueIndex;
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			VK_CHECK_RESULT(vkCreateCommandPool(m_deviceInfo->Device, &cmdPoolInfo, nullptr, &m_cmdPool[i]));

			VkCommandBufferAllocateInfo cmdAllocInfo{};

			cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdAllocInfo.commandBufferCount = 1;
			cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdAllocInfo.commandPool = m_cmdPool[i];

			VK_CHECK_RESULT(vkAllocateCommandBuffers(m_deviceInfo->Device, &cmdAllocInfo, &m_cmdBuffer[i]));
		}

		//Semaphore Creation
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreateSemaphore(m_deviceInfo->Device, &semaphoreInfo, nullptr, &m_transfer_complete_semaphore));

		//Fence Creation
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VK_CHECK_RESULT(vkCreateFence(m_deviceInfo->Device, &fenceInfo, nullptr, &m_transferFence));
	}

	void AsynchronousLoader::Update()
	{
		//std::cout << "Async Update has been Invoked" << std::endl;


		//file Process
		if (m_fileloadRequests.size() > 0)
		{
			if (vkGetFenceStatus(m_deviceInfo->Device, m_transferFence) != VK_SUCCESS)
			{
				return;
			}

			vkResetFences(m_deviceInfo->Device, 1, &m_transferFence);

			FileLoadRequest loadreq = m_fileloadRequests.front();
			m_fileloadRequests.pop();

			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
			Texture texture{};
			std::cout << "Current Frame =>" << m_deviceInfo->current_frame << std::endl;
			VkCommandPool& cmdpool = m_cmdPool[m_deviceInfo->current_frame];
			VkCommandBuffer& cmdBuffer = m_cmdBuffer[m_deviceInfo->current_frame];
			texture.load_Texture(loadreq.path, m_deviceInfo,cmdpool, cmdBuffer, m_transfer_complete_semaphore, m_transferFence, format, false);

			
		
			m_materials->m_textures.push_back(texture);
		}
	}

	void AsynchronousLoader::request_texture_request(std::string filename)
	{
		FileLoadRequest file_request{};
		file_request.path = filename;
		file_request.texure = true;

		m_fileloadRequests.push(file_request);
	}

	void AsynchronousLoader::Shutdown()
	{
		for (int i = 0; i < m_cmdPool.size(); i++)
		{
			vkDestroyCommandPool(m_deviceInfo->Device, m_cmdPool[i], nullptr);
		}

		vkDestroySemaphore(m_deviceInfo->Device, m_transfer_complete_semaphore, nullptr);
		vkDestroyFence(m_deviceInfo->Device, m_transferFence, nullptr);
	}
}