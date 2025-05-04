#include "Buffer.h"

namespace graphics
{
	VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(m_device, memory, offset, size, 0, &mapped);
	}

	void Buffer::Unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(m_device, memory);
			mapped = nullptr;
		}
	}

	VkResult Buffer::bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(m_device, buffer, memory, offset);
	}

	void Buffer::copyTo(void* data, VkDeviceSize size)
	{
		assert(mapped);
		memcpy(mapped, data, size);
	}


	void Buffer::Allocate(VkPhysicalDevice PhysicalDevice, VkDevice device, void* data, int size, VkBufferUsageFlags usageFlag, VkMemoryPropertyFlags memProps)
	{
		BufferSize = size;
		m_device = device;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = BufferSize;
		bufferInfo.usage = usageFlag;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer));

		VkMemoryRequirements memReqs{};
		vkGetBufferMemoryRequirements(m_device, buffer, &memReqs);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memReqs.memoryTypeBits, memProps, PhysicalDevice);

		VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &memory));

		if (data != nullptr)
		{
			vkMapMemory(m_device, memory, 0, BufferSize, 0, &mapped);
			memcpy(mapped, data, size);
			vkUnmapMemory(m_device, memory);
		}

		vkBindBufferMemory(m_device, buffer, memory, 0);
		
	}

	void Buffer::Destroy()
	{
		if (buffer)
		{
			vkDestroyBuffer(m_device, buffer, nullptr);
		}

		if (memory)
		{
			vkFreeMemory(m_device, memory, nullptr);
		}
	}
}