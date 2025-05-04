#include "Command_Buffer.h"
/*Command Buffer can take care of dealing with adding bunch of data to the descriptor sets
* It can take care of recording and producing command Buffers
* It can also provide modules for creating synchronization objects
* 
*/

namespace graphics
{
	void CommandBuffer::Init(DeviceInfo* deviceInfo, OffscreenData* offScreenData, skybox* skyBox)
	{
		m_deviceInfo = deviceInfo;
		m_offScreenData = offScreenData;
		m_skyBox = skyBox;

		//CommandPool Creation
		QueueFamily queueFamilyIndex = QueueFamily::findQueueFamily(m_deviceInfo->PhysicalDevice);
		VkCommandPoolCreateInfo cmdPoolInfo{};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.pNext = nullptr;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex.graphicsQueueIndex.value();
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK_RESULT(vkCreateCommandPool(deviceInfo->Device, &cmdPoolInfo, nullptr, &m_deviceInfo->commandPool));

		InitDescriptors();
		CreateImmutableSamplerDescriptorSet();
		CreateBindlessDescriptor();
		Prepare_PipelineLayout();
		CreatePrimitiveSynchronizationObject();
		Allocate_CommandBuffer();
		CreatePipelineCache();
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
		std::array<VkDescriptorSetLayout, 4> setLayouts = {m_deviceInfo->descriptorSetLayout, m_deviceInfo->skyboxDescriptorSetLayout, 
			m_deviceInfo->bindless.setLayout, m_deviceInfo->sampler_descriptors.setLayout};

		VkPushConstantRange pushConstant{};
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstant.size = sizeof(PushConstant);
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

		//SkyBox DescriptorSetLayout
		std::vector<VkDescriptorSetLayoutBinding> skyboxLayoutBinding =
		{
			initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
		};

		setLayoutCreateInfo.bindingCount = static_cast<uint32_t>(skyboxLayoutBinding.size());
		setLayoutCreateInfo.pBindings = skyboxLayoutBinding.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_deviceInfo->Device, &setLayoutCreateInfo, nullptr, &m_deviceInfo->skyboxDescriptorSetLayout));


		/*Descriptor pool is responsible for allocating the descriptor sets according to their size and Type*/
		std::array<VkDescriptorPoolSize, 4> DescriptorPoolSize{};

		//We can carry one per object // ToDo -: Update this count when you finish the work in model class regarding Descriptors
		
		DescriptorPoolSize[0].descriptorCount = 3;
		DescriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		// Same for this content as well we carry one per object Texture
		DescriptorPoolSize[1].descriptorCount = 3;
		DescriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;


		DescriptorPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorPoolSize[2].descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;

		DescriptorPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorPoolSize[3].descriptorCount = 1;


		VkDescriptorPoolCreateInfo PoolInfo{};
		PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		PoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		PoolInfo.poolSizeCount = static_cast<uint32_t>(DescriptorPoolSize.size());
		PoolInfo.pPoolSizes = DescriptorPoolSize.data();
		PoolInfo.maxSets = 5;

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


		//Create Skybox DescriptorSet 
		const VkDescriptorSetAllocateInfo skyBoxAllocInfo = initializer::descriptorSetAllocInfo(&m_deviceInfo->skyboxDescriptorSetLayout, m_deviceInfo->DescriptorPool, 1);
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_deviceInfo->Device, &skyBoxAllocInfo, &m_skyBox->descriptorInfo.descriptorSet));

		VkWriteDescriptorSet skyBoxwriteDescriptor = initializer::writeDescriptorSet(m_skyBox->descriptorInfo.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			0, &m_skyBox->descriptorInfo.imageInfo);
		vkUpdateDescriptorSets(m_deviceInfo->Device, 1, &skyBoxwriteDescriptor, 0, nullptr);
	}

	void CommandBuffer::CreatePipelineCache()
	{
		std::ifstream file("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Temp/pipeline_cache.bin", std::ios::binary | std::ios::ate);
		std::vector<char> cacheData;

		if (file.is_open())
		{
			size_t fileSize = file.tellg();
			file.seekg(0, std::ios::beg);
			cacheData.resize(fileSize);
			file.read(cacheData.data(), fileSize);
			file.close();
		}

		bool cache_enable = false;

		if (cacheData.size() > 0)
		{
			VkPipelineCacheHeaderVersionOne cache_header;
			std::memcpy(&cache_header, cacheData.data(), sizeof(VkPipelineCacheHeaderVersionOne));

			VkPhysicalDeviceProperties props{};
			vkGetPhysicalDeviceProperties(m_deviceInfo->PhysicalDevice, &props);

			if (cache_header.deviceID == props.deviceID && cache_header.vendorID == props.vendorID && memcmp(cache_header.pipelineCacheUUID, props.pipelineCacheUUID, VK_UUID_SIZE) == 0)
			{
				cache_enable = true;
			}
		}


		VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCreateInfo.initialDataSize = (cache_enable) ? cacheData.size() : 0;
		pipelineCacheCreateInfo.pInitialData = (cache_enable) ? cacheData.data() : nullptr;

		VK_CHECK_RESULT(vkCreatePipelineCache(m_deviceInfo->Device, &pipelineCacheCreateInfo, nullptr, &m_deviceInfo->pipelineCache));


	}

	void CommandBuffer::SavePipelineCache()
	{
		size_t cacheSize = 0;
		vkGetPipelineCacheData(m_deviceInfo->Device, m_deviceInfo->pipelineCache, &cacheSize, nullptr);

		if (cacheSize == 0)
		{
			throw std::runtime_error("Cache data is empty");
		}

		std::vector<char> cacheData(cacheSize);
		vkGetPipelineCacheData(m_deviceInfo->Device, m_deviceInfo->pipelineCache, &cacheSize, cacheData.data());


		std::ofstream file("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Temp/pipeline_cache.bin", std::ios::binary);

		if (file.is_open())
		{
			file.write(cacheData.data(), cacheSize);
			file.close();
		}
		else
		{
			std::cerr << "Failed to save pipeline cache to file!" << std::endl;
		}
	}

	void CommandBuffer::CreateImmutableSamplerDescriptorSet()
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		VkFilter filter = VK_FILTER_LINEAR;

		make_filters_valid(m_deviceInfo->PhysicalDevice, format, &filter, nullptr);

		// The common case for bindless is to have an array of sampled images, not combined image sampler.
		// It is more efficient to use a single sampler instead, and we can just use a single immutable sampler for this purpose.
		// Create the sampler, descriptor set layout and allocate an immutable descriptor set
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.minFilter = filter;
		samplerInfo.magFilter = filter;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
		
		VkPhysicalDeviceFeatures deviceFeature;
		vkGetPhysicalDeviceFeatures(m_deviceInfo->PhysicalDevice, &deviceFeature);
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(m_deviceInfo->PhysicalDevice, &deviceProperties);

		if (deviceFeature.samplerAnisotropy)
		{
			samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
			samplerInfo.anisotropyEnable = VK_TRUE;
		}
		else
		{
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.maxAnisotropy = 1.0f;
		}

		VK_CHECK_RESULT(vkCreateSampler(m_deviceInfo->Device, &samplerInfo, nullptr, &m_deviceInfo->sampler_descriptors.sampler));

		VkDescriptorSetLayoutBinding layoutBinding = initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		layoutBinding.pImmutableSamplers = &m_deviceInfo->sampler_descriptors.sampler;

		VkDescriptorSetLayoutCreateInfo setLayoutCreateinfo = initializer::descriptorSetLayoutCreateInfo(&layoutBinding, 1);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_deviceInfo->Device, &setLayoutCreateinfo, nullptr, &m_deviceInfo->sampler_descriptors.setLayout));

		//Descriptor Pool
		VkDescriptorPoolSize poolSize = initializer::descriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1);
		VkDescriptorPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.maxSets = 1;
		poolCreateInfo.poolSizeCount = 1;
		poolCreateInfo.pPoolSizes = &poolSize;
		VK_CHECK_RESULT(vkCreateDescriptorPool(m_deviceInfo->Device, &poolCreateInfo, nullptr, &m_deviceInfo->sampler_descriptors.descriptor_Pool));

		//DescriptorSet
		VkDescriptorSetAllocateInfo allocInfo = initializer::descriptorSetAllocInfo(&m_deviceInfo->sampler_descriptors.setLayout, m_deviceInfo->sampler_descriptors.descriptor_Pool, 1);

		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_deviceInfo->Device, &allocInfo, &m_deviceInfo->sampler_descriptors.descriptorSet));
	}


	void CommandBuffer::CreateBindlessDescriptor()
	{
		uint32_t descriptorCount = m_deviceInfo->indexingProperties.maxDescriptorSetUpdateAfterBindSampledImages - 100;
		const uint32_t NumDescriptorsStreaming = 2048;

		//Set Layout Creation
		VkDescriptorSetLayoutBinding setLayoutBinding = initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 0, descriptorCount);
		VkDescriptorSetLayoutCreateInfo setLayoutInfo = initializer::descriptorSetLayoutCreateInfo(&setLayoutBinding, 1);

		setLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		//Descriptor indexing Flags
		const VkDescriptorBindingFlagsEXT flags =
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlags{};
		bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		bindingFlags.pBindingFlags = &flags;
		bindingFlags.bindingCount = 1;
		setLayoutInfo.pNext = &bindingFlags;

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_deviceInfo->Device, &setLayoutInfo, nullptr, &m_deviceInfo->bindless.setLayout));

		//Descriptor Pool Creation
		uint32_t poolCount = NumDescriptorsStreaming;

		VkDescriptorPoolSize descriptorPoolSize = initializer::descriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, poolCount);
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = 1;
		descriptorPoolInfo.pPoolSizes = &descriptorPoolSize;
		descriptorPoolInfo.maxSets = 2;

		descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;

		VK_CHECK_RESULT(vkCreateDescriptorPool(m_deviceInfo->Device, &descriptorPoolInfo, nullptr, &m_deviceInfo->bindless.descriptor_Pool));

		//DescriptorSet Creation

		VkDescriptorSetAllocateInfo allocInfo = initializer::descriptorSetAllocInfo(&m_deviceInfo->bindless.setLayout, m_deviceInfo->bindless.descriptor_Pool, 1);
		//for each descriptor set we allocate, we can describe how large the descriptor array should be.
		VkDescriptorSetVariableDescriptorCountAllocateInfoEXT  variableInfo{};
		variableInfo.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
		variableInfo.descriptorSetCount = 1;
		allocInfo.pNext					= &variableInfo;

		variableInfo.pDescriptorCounts = &NumDescriptorsStreaming;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_deviceInfo->Device, &allocInfo, &m_deviceInfo->bindless.update_after_bind_descriptorSet));

	}

	void CommandBuffer::shutDown()
	{
		UniformBuffer.Destroy();
		offscreenUniformBuffer.Destroy();
		vkDestroyPipelineCache(m_deviceInfo->Device, m_deviceInfo->pipelineCache, nullptr);
		vkDestroyPipelineLayout(m_deviceInfo->Device, m_deviceInfo->pipelineLayout, nullptr);
		vkDestroyCommandPool(m_deviceInfo->Device, m_deviceInfo->commandPool, nullptr);
		vkDestroyDescriptorPool(m_deviceInfo->Device, m_deviceInfo->DescriptorPool, nullptr);
		vkDestroyDescriptorPool(m_deviceInfo->Device, m_deviceInfo->bindless.descriptor_Pool, nullptr);
		vkDestroyDescriptorPool(m_deviceInfo->Device, m_deviceInfo->sampler_descriptors.descriptor_Pool, nullptr);
		vkDestroyDescriptorSetLayout(m_deviceInfo->Device, m_deviceInfo->descriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_deviceInfo->Device, m_deviceInfo->skyboxDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_deviceInfo->Device, m_deviceInfo->bindless.setLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_deviceInfo->Device, m_deviceInfo->sampler_descriptors.setLayout, nullptr);
		vkDestroySampler(m_deviceInfo->Device, m_deviceInfo->sampler_descriptors.sampler, nullptr);

		for (int i = 0; i < m_deviceInfo->MAX_FRAME_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_deviceInfo->Device, m_deviceInfo->imageAvailableSemaphore[i], nullptr);
			vkDestroySemaphore(m_deviceInfo->Device, m_deviceInfo->renderFinishedSemaphore[i], nullptr);
			vkDestroyFence(m_deviceInfo->Device, m_deviceInfo->inFlightFence[i], nullptr);
		}
	}


	
}