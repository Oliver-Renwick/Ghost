#include "_CommandBuffer.h"


namespace graphics
{

	void CommandBuffer::begin()
	{
		if (!is_recording)
		{
			VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);

			is_recording = true;
		}
	}

	void CommandBuffer::end()
	{
		if (is_recording)
		{
			vkEndCommandBuffer(m_CommandBuffer);
			is_recording = false;
		}
	}

	void CommandBuffer::begin_secondary()
	{
		if (!is_recording)
		{
			VkCommandBufferInheritanceInfo inheritanceInfo{};
			inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritanceInfo.renderPass = m_deviceInfo->renderPass;
			inheritanceInfo.subpass = 0;
			inheritanceInfo.framebuffer = m_deviceInfo->frameBuffers[m_deviceInfo->current_frame];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			beginInfo.pInheritanceInfo = &inheritanceInfo;

			vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);

			is_recording = false;

		}
	}

	void CommandBuffer::set_viewport()
	{
		VkViewport viewPort{};
		viewPort.x = 0.0f;
		viewPort.y = 0.0f;
		viewPort.minDepth = 0.0f;
		viewPort.maxDepth = 1.0f;
		viewPort.width = static_cast<float>(m_deviceInfo->swapchainextent.width);
		viewPort.height = static_cast<float>(m_deviceInfo->swapchainextent.height);

		vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewPort);
	}

	void CommandBuffer::set_scissors()
	{
		VkRect2D vk_scissors{};

		vk_scissors.offset.x = 0;
		vk_scissors.offset.y = 0;
		vk_scissors.extent = m_deviceInfo->swapchainextent;

		vkCmdSetScissor(m_CommandBuffer, 0, 1, &vk_scissors);
	}

	void CommandBuffer::clear(float red, float green, float blue, float alpha)
	{
		m_clearVal[0].color = { red, green, blue, alpha };
	}

	void CommandBuffer::clear_depth_stencil(float depth, uint8_t value)
	{
		m_clearVal[1].depthStencil.depth = depth;
		m_clearVal[1].depthStencil.stencil = value;
	}



	void CommandBufferManager::Init(DeviceInfo* deviceInfo, uint32_t numThreads)
	{
		assert(deviceInfo);
		m_deviceInfo = deviceInfo;

		num_pool_per_frame = numThreads;

		//Create pools => numFrames * numThreads
		const uint32_t totalPools = num_pool_per_frame * m_deviceInfo->MAX_FRAME_IN_FLIGHT;
		m_commandpools.resize(totalPools);
		//reserve for the used buffer index
		used_buffer.resize(totalPools);

		for (uint32_t i = 0; i < totalPools; i++)
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = m_deviceInfo->graphicsQueueIndex;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			vkCreateCommandPool(m_deviceInfo->Device, &poolInfo, nullptr, &m_commandpools[i]);

			used_buffer[i] = 0;
		}

		//Create Command Buffer => pools * buffers per pool
		const uint32_t total_buffers = totalPools * num_command_buffer_per_pool;
		m_commandbuffers.reserve(total_buffers);

		for (uint32_t i = 0; i < total_buffers; i++)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

			const uint32_t frame_index = i / (num_command_buffer_per_pool * num_pool_per_frame);
			const uint32_t thread_index = (i / num_command_buffer_per_pool) % num_pool_per_frame;
			const uint32_t pool_index = pool_from_indices(frame_index, thread_index);

			allocInfo.commandPool = m_commandpools[pool_index];
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			CommandBuffer& cmdBuffer = m_commandbuffers[i];
			vkAllocateCommandBuffers(m_deviceInfo->Device, &allocInfo, &cmdBuffer.m_CommandBuffer);
		}
	}

	void CommandBufferManager::resetPool(uint32_t frameIndex)
	{
		for (uint32_t i = 0; i < num_pool_per_frame; i++)
		{
			const uint32_t pool_index = pool_from_indices(frameIndex, i);
			vkResetCommandPool(m_deviceInfo->Device, m_commandpools[pool_index], 0);

			used_buffer[pool_index] = 0;
		}
	}

	//VkCommandBuffer CommandBufferManager::get_command_buffer(uint32_t frmaeIndex, uint32_t thread_index, bool begin)
	//{
	//	VkCommandBuffer cmdBuffer{};
	//	return cmdBuffer;
	//	// Plan this section!!
	//}

	void CommandBufferManager::shutdown()
	{
		const uint32_t total_pools = num_pool_per_frame * m_deviceInfo->MAX_FRAME_IN_FLIGHT;

		//Buffers get freed with pools
		for (uint32_t i = 0; i < total_pools; i++)
		{
			vkDestroyCommandPool(m_deviceInfo->Device, m_commandpools[i], nullptr);
		}
	}

	uint32_t CommandBufferManager::pool_from_indices(uint32_t frameIndex, uint32_t threadIndex)
	{
		return (frameIndex * num_pool_per_frame) + threadIndex;
	}
}