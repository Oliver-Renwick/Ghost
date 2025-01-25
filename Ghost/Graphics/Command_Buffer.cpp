#include "Command_Buffer.h"
/*Command Buffer can take care of dealing with adding bunch of data to the descriptor sets
* It can take care of recording and producing command Buffers
* It can also provide modules for creating synchronization objects
* 
*/

namespace graphics
{
	void CommandBuffer::Init(DeviceInfo* deviceInfo, OffscreenData* offScreenData)
	{
		m_deviceInfo = deviceInfo;
		m_offScreenData = offScreenData;

		//CommandPool Creation
		QueueFamily queueFamilyIndex = QueueFamily::findQueueFamily(m_deviceInfo->PhysicalDevice);
		VkCommandPoolCreateInfo cmdPoolInfo{};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.pNext = nullptr;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex.graphicsQueueIndex.value();
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK_RESULT(vkCreateCommandPool(deviceInfo->Device, &cmdPoolInfo, nullptr, &m_deviceInfo->commandPool));

		InitDescriptors();
		Prepare_PipelineLayout();
		CreatePrimitiveSynchronizationObject();
		Allocate_CommandBuffer();
	}

	void CommandBuffer::CreatePrimitiveSynchronizationObject()
	{
		int MaxFrames = m_deviceInfo->MAX_FRAME_IN_FLIGHT;
		m_deviceInfo->imageAvailableSemaphore.resize(MaxFrames);
		m_deviceInfo->renderFinishedSemaphore.resize(MaxFrames);
		m_deviceInfo->inFlightFence.resize(MaxFrames);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0; i < MaxFrames; i++)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(m_deviceInfo->Device, &semaphoreInfo, nullptr, &m_deviceInfo->imageAvailableSemaphore[i]));
			VK_CHECK_RESULT(vkCreateSemaphore(m_deviceInfo->Device, &semaphoreInfo, nullptr, &m_deviceInfo->renderFinishedSemaphore[i]));
			VK_CHECK_RESULT(vkCreateFence(m_deviceInfo->Device, &fenceInfo, nullptr, &m_deviceInfo->inFlightFence[i]));
		}
	}

	void CommandBuffer::Allocate_CommandBuffer()
	{
		int Frames = m_deviceInfo->MAX_FRAME_IN_FLIGHT;
		VkCommandPool& cmdPool = m_deviceInfo->commandPool;
		m_deviceInfo->_CommandBuffers.resize(Frames);

		VkCommandBufferAllocateInfo cmdAllocInfo{};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAllocInfo.commandPool = cmdPool;
		cmdAllocInfo.commandBufferCount = static_cast<uint32_t>(m_deviceInfo->_CommandBuffers.size());

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_deviceInfo->Device, &cmdAllocInfo, m_deviceInfo->_CommandBuffers.data()));
	}

	void CommandBuffer::Prepare_PipelineLayout()
	{
		std::array<VkDescriptorSetLayout, 1> setLayouts = {m_deviceInfo->descriptorSetLayout};

		VkPushConstantRange pushConstant{};
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstant.size = sizeof(glm::mat4);
		pushConstant.offset = 0;

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstant;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		layoutInfo.pSetLayouts = setLayouts.data();

		VK_CHECK_RESULT(vkCreatePipelineLayout(m_deviceInfo->Device, &layoutInfo, nullptr, &m_deviceInfo->pipelineLayout));
	}

	void CommandBuffer::InitDescriptors()
	{
		/*Descriptor set layout which is responsible for set layout and bindings in shaders*/
		std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBinding{};

		setLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setLayoutBinding[0].binding = 0;
		setLayoutBinding[0].descriptorCount = 1;
		setLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		setLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setLayoutBinding[1].binding = 1;
		setLayoutBinding[1].descriptorCount = 1;
		setLayoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo{};
		setLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBinding.size());
		setLayoutCreateInfo.pBindings = setLayoutBinding.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_deviceInfo->Device, &setLayoutCreateInfo, nullptr, &m_deviceInfo->descriptorSetLayout));


		/*Descriptor pool is responsible for allocating the descriptor sets according to their size and Type*/
		std::array<VkDescriptorPoolSize, 3> DescriptorPoolSize{};

		//We can carry one per object // ToDo -: Update this count when you finish the work in model class regarding Descriptors
		
		DescriptorPoolSize[0].descriptorCount = 3;
		DescriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		// Same for this content as well we carry one per object Texture
		DescriptorPoolSize[1].descriptorCount = 3;
		DescriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;


		DescriptorPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorPoolSize[2].descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;


		VkDescriptorPoolCreateInfo PoolInfo{};
		PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		PoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		PoolInfo.poolSizeCount = static_cast<uint32_t>(DescriptorPoolSize.size());
		PoolInfo.pPoolSizes = DescriptorPoolSize.data();
		PoolInfo.maxSets = 4;

		VK_CHECK_RESULT(vkCreateDescriptorPool(m_deviceInfo->Device, &PoolInfo, nullptr, &m_deviceInfo->DescriptorPool));


		/*ToDO :- Finish Descriptor Sets When finishing the model class*/
	}

	void CommandBuffer::CreateDescriptorSet()
	{

		//Image Descriptor 
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = m_offScreenData->depthView;
		imageInfo.sampler = m_offScreenData->depthsampler;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;


		//Create Shadow Uniform Buffer
		int size = sizeof(UboInfoOffscreen);
		offscreenUniformBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, nullptr, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		offscreenUniformBuffer.map();

		VkDescriptorSetAllocateInfo descriptorSetInfo{};
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.descriptorPool = m_deviceInfo->DescriptorPool;
		descriptorSetInfo.pSetLayouts = &m_deviceInfo->descriptorSetLayout;
		descriptorSetInfo.descriptorSetCount = 1;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_deviceInfo->Device, &descriptorSetInfo, &m_offScreenData->descriptorSet));
		
		VkDescriptorBufferInfo BufferInfo{};
		BufferInfo.buffer = offscreenUniformBuffer.buffer;
		BufferInfo.offset = 0;
		BufferInfo.range = sizeof(UboInfoOffscreen);

		VkWriteDescriptorSet offScreenWriteDescriptorSet{};
		offScreenWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		offScreenWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		offScreenWriteDescriptorSet.dstBinding = 0;
		offScreenWriteDescriptorSet.descriptorCount = 1;
		offScreenWriteDescriptorSet.pBufferInfo = &BufferInfo;
		offScreenWriteDescriptorSet.dstSet = m_offScreenData->descriptorSet;

		vkUpdateDescriptorSets(m_deviceInfo->Device, 1, &offScreenWriteDescriptorSet, 0, nullptr);

		//Create Scene Uniform Buffer
		size = sizeof(UboInfo);
		UniformBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, nullptr, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		UniformBuffer.map();

		VkResult res = vkAllocateDescriptorSets(m_deviceInfo->Device, &descriptorSetInfo, &m_deviceInfo->descriptorSet);

		VkDescriptorBufferInfo descriptorBufferInfo{};
		descriptorBufferInfo.buffer = UniformBuffer.buffer;
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = sizeof(UboInfo);

		std::array<VkWriteDescriptorSet, 2> writeDescriptor{};
		writeDescriptor[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptor[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptor[0].dstBinding = 0;
		writeDescriptor[0].descriptorCount = 1;
		writeDescriptor[0].pBufferInfo = &descriptorBufferInfo;
		writeDescriptor[0].dstSet = m_deviceInfo->descriptorSet;

		writeDescriptor[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptor[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptor[1].dstBinding = 1;
		writeDescriptor[1].descriptorCount = 1;
		writeDescriptor[1].pImageInfo = &imageInfo;
		writeDescriptor[1].dstSet = m_deviceInfo->descriptorSet;

		vkUpdateDescriptorSets(m_deviceInfo->Device, static_cast<uint32_t>(writeDescriptor.size()), writeDescriptor.data(), 0, nullptr);



		//Create Debug Descriptor
		res = vkAllocateDescriptorSets(m_deviceInfo->Device, &descriptorSetInfo, &m_offScreenData->debugDescriptor);


		writeDescriptor[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptor[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptor[0].dstBinding = 0;
		writeDescriptor[0].descriptorCount = 1;
		writeDescriptor[0].pBufferInfo = &descriptorBufferInfo;
		writeDescriptor[0].dstSet = m_offScreenData->debugDescriptor;

		writeDescriptor[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptor[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptor[1].dstBinding = 1;
		writeDescriptor[1].descriptorCount = 1;
		writeDescriptor[1].pImageInfo = &imageInfo;
		writeDescriptor[1].dstSet = m_offScreenData->debugDescriptor;

		vkUpdateDescriptorSets(m_deviceInfo->Device, static_cast<uint32_t>(writeDescriptor.size()), writeDescriptor.data(), 0, nullptr);

	}

	void CommandBuffer::shutDown()
	{
		UniformBuffer.Destroy();
		offscreenUniformBuffer.Destroy();
		vkDestroyPipelineLayout(m_deviceInfo->Device, m_deviceInfo->pipelineLayout, nullptr);
		vkDestroyCommandPool(m_deviceInfo->Device, m_deviceInfo->commandPool, nullptr);
		vkDestroyDescriptorPool(m_deviceInfo->Device, m_deviceInfo->DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_deviceInfo->Device, m_deviceInfo->descriptorSetLayout, nullptr);

		for (int i = 0; i < m_deviceInfo->MAX_FRAME_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_deviceInfo->Device, m_deviceInfo->imageAvailableSemaphore[i], nullptr);
			vkDestroySemaphore(m_deviceInfo->Device, m_deviceInfo->renderFinishedSemaphore[i], nullptr);
			vkDestroyFence(m_deviceInfo->Device, m_deviceInfo->inFlightFence[i], nullptr);
		}
	}
}