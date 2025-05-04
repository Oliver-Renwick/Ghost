#pragma once
#include "Tools.h"
#include "Vulkan_Init.h"
#include "Buffer.h"

namespace graphics
{

	struct CommandBuffer
	{
		DeviceInfo*     m_deviceInfo = nullptr;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkClearValue    m_clearVal[2]; 
		bool			is_recording;

		void		    begin();
		void			end();
		void			begin_secondary();
		void			set_viewport();
		void			set_scissors();
		void			clear(float red, float green, float blue, float alpha);
		void            clear_depth_stencil(float depth, uint8_t value);
	};

	struct CommandBufferManager
	{
		DeviceInfo* m_deviceInfo = nullptr;
		std::vector<VkCommandPool>	 m_commandpools;
		std::vector<CommandBuffer>   m_commandbuffers;
		std::vector<uint32_t>		 used_buffer;
		uint32_t					 num_pool_per_frame = 0;
		uint32_t					 num_command_buffer_per_pool = 3;


		void     Init(DeviceInfo* deviceInfo, uint32_t numThreads);
		uint32_t pool_from_indices(uint32_t frameIndex, uint32_t threadIndex);
		void	 resetPool(uint32_t frameIndex);
		void	 shutdown();
		CommandBuffer get_command_buffer(uint32_t frmaeIndex, uint32_t thread_index, bool begin);
	};

}