#include "Skybox.h"

namespace graphics
{
	void skybox::createCubemap(std::string filename, VkFormat format, DeviceInfo* deviceInfo)
	{
		m_deviceInfo = deviceInfo;
		assert(m_deviceInfo != nullptr);

		if (!initializer::fileExist(filename))
		{
			throw std::runtime_error("Unable to open the file");
		}

		//Create Cube
		CreateCube();

		//Loading Texture

		ktxTexture* ktx_Texture;
		ktxResult result;

		result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_Texture);

		assert(result == KTX_SUCCESS);

		m_width = ktx_Texture->baseWidth;
		m_height = ktx_Texture->baseHeight;
		m_mipLevels = ktx_Texture->numLevels;


		ktx_uint8_t* textureData = ktxTexture_GetData(ktx_Texture);
		ktx_size_t textureSize = ktxTexture_GetDataSize(ktx_Texture);


		// staging buffer for host

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.size = textureSize;

		VK_CHECK_RESULT(vkCreateBuffer(m_deviceInfo->Device, &bufferInfo, nullptr, &stagingBuffer));

		VkMemoryRequirements memReq{};
		VkMemoryAllocateInfo allocInfo{};

		vkGetBufferMemoryRequirements(m_deviceInfo->Device, stagingBuffer, &memReq);

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_deviceInfo->PhysicalDevice);


		VK_CHECK_RESULT(vkAllocateMemory(m_deviceInfo->Device, &allocInfo, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(m_deviceInfo->Device, stagingBuffer, stagingMemory, 0));

		//update data in the staging buffer
		uint8_t* data;
		vkMapMemory(m_deviceInfo->Device, stagingMemory, 0, memReq.size, 0, (void**)&data);
		memcpy(data, textureData, textureSize);
		vkUnmapMemory(m_deviceInfo->Device, stagingMemory);

		//Create Image
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.extent.width = m_width;
		imageInfo.extent.height = m_height;
		imageInfo.extent.depth = 1;
		imageInfo.format = format ;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.mipLevels = m_mipLevels;

		//count as many as Cubmap faces
		imageInfo.arrayLayers = 6;

		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VK_CHECK_RESULT(vkCreateImage(m_deviceInfo->Device, &imageInfo, nullptr, &m_image));

		vkGetImageMemoryRequirements(m_deviceInfo->Device, m_image, &memReq);

		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_deviceInfo->PhysicalDevice);

		VK_CHECK_RESULT(vkAllocateMemory(m_deviceInfo->Device, &allocInfo, nullptr, &m_imageMemory));
		VK_CHECK_RESULT(vkBindImageMemory(m_deviceInfo->Device, m_image, m_imageMemory, 0));

		//Copying Time
		std::vector<VkBufferImageCopy> bufferCopyRegions{};

		for (uint32_t face = 0; face < 6; face++)
		{
			for (uint32_t levels = 0; levels < m_mipLevels; levels++)
			{
				ktx_size_t offset = 0;
				ktx_error_code_e res = ktxTexture_GetImageOffset(ktx_Texture, levels, 0, face, &offset);

				VkBufferImageCopy copyRegion{};
				copyRegion.imageSubresource.baseArrayLayer = face;
				copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.imageSubresource.mipLevel = levels;
				copyRegion.imageSubresource.layerCount = 1;
				copyRegion.imageExtent.width = ktx_Texture->baseWidth >> levels;
				copyRegion.imageExtent.height = ktx_Texture->baseHeight >> levels;
				copyRegion.imageExtent.depth = 1;
				copyRegion.bufferOffset = offset;

				bufferCopyRegions.push_back(copyRegion);

			}
		}

		//Layout Transitioning

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = m_mipLevels;
		subresourceRange.layerCount = 6;


		VkCommandBuffer CopyCmd;

		VkCommandBufferAllocateInfo cmdAllocInfo{};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.commandPool = m_deviceInfo->commandPool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_deviceInfo->Device, &cmdAllocInfo, &CopyCmd));

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK_RESULT(vkBeginCommandBuffer(CopyCmd, &beginInfo));

		initializer::setImageLayout(
			CopyCmd,
			m_image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);

		vkCmdCopyBufferToImage(
			CopyCmd,
			stagingBuffer,
			m_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data()
		);

		m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		initializer::setImageLayout(
			CopyCmd,
			m_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			m_imageLayout,
			subresourceRange,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);


		vkEndCommandBuffer(CopyCmd);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CopyCmd;

		vkQueueSubmit(m_deviceInfo->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_deviceInfo->GraphicsQueue);
		vkFreeCommandBuffers(m_deviceInfo->Device, m_deviceInfo->commandPool, 1, &CopyCmd);


		//Sampler Create Info
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(m_mipLevels);
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VkPhysicalDeviceFeatures deviceFeatures{};
		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceFeatures(m_deviceInfo->PhysicalDevice, &deviceFeatures);
		vkGetPhysicalDeviceProperties(m_deviceInfo->PhysicalDevice, &deviceProperties);

		if (deviceFeatures.samplerAnisotropy)
		{
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
		}

		else
		{
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.maxAnisotropy = 1.0f;
		}

		VK_CHECK_RESULT(vkCreateSampler(m_deviceInfo->Device, &samplerInfo, nullptr, &m_sampler));

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = format;
		viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		//For all 6 faces
		viewInfo.subresourceRange.layerCount = 6;
		viewInfo.subresourceRange.levelCount = m_mipLevels;

		viewInfo.image = m_image;

		VK_CHECK_RESULT(vkCreateImageView(m_deviceInfo->Device, &viewInfo, nullptr, &m_imageView));

		//cleanup
		vkDestroyBuffer(m_deviceInfo->Device, stagingBuffer, nullptr);
		vkFreeMemory(m_deviceInfo->Device, stagingMemory, nullptr);
		ktxTexture_Destroy(ktx_Texture);

		UpdateDescriptor();

	}

	void skybox::preparePipeline()
	{
		auto vertexShader = readfile("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Skybox_vert.spv");
		auto fragmentShader = readfile("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/Skybox_frag.spv");

		VkShaderModule vertexModule = createShaderModule(vertexShader, m_deviceInfo->Device);
		VkShaderModule fragmentModule = createShaderModule(fragmentShader, m_deviceInfo->Device);

		VkPipelineShaderStageCreateInfo vertexPipelineStage{};
		vertexPipelineStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexPipelineStage.pName = "main";
		vertexPipelineStage.module = vertexModule;
		vertexPipelineStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineShaderStageCreateInfo fragmentPipelineStage{};
		fragmentPipelineStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentPipelineStage.pName = "main";
		fragmentPipelineStage.module = fragmentModule;
		fragmentPipelineStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPipelineShaderStageCreateInfo shaderstage[2] = { vertexPipelineStage, fragmentPipelineStage };


		VkVertexInputBindingDescription vertexInputBindingDescription = vert::GetBindingDescription();
		std::array<VkVertexInputAttributeDescription, 5> attributeDescription = vert::GetAttributeDescription();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblystateCI = initializer::inputAssemblySate(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0);
		VkPipelineRasterizationStateCreateInfo rasterizationstateCI = initializer::razterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState colorBlendAttacments = initializer::colorBlendAttachment(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendCI = initializer::colorBlendInfo(1, &colorBlendAttacments);
		VkPipelineDepthStencilStateCreateInfo depthstencilCI = initializer::depthStencilInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewPortInfo = initializer::viewportStateInfo(1, 1, nullptr, nullptr);
		VkPipelineMultisampleStateCreateInfo multiSampleCI = initializer::MultiSampleStateInfo(m_deviceInfo->msaaSamples);
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCI = initializer::dynamicStateInfo(dynamicStateEnables);

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.renderPass = m_deviceInfo->renderPass;
		graphicsPipelineCreateInfo.layout = m_deviceInfo->pipelineLayout;
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblystateCI;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizationstateCI;
		graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCI;
		graphicsPipelineCreateInfo.pDepthStencilState = &depthstencilCI;
		graphicsPipelineCreateInfo.pViewportState = &viewPortInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multiSampleCI;
		graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCI;
		graphicsPipelineCreateInfo.stageCount = 2;
		graphicsPipelineCreateInfo.pStages = shaderstage;
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;

		rasterizationstateCI.cullMode = VK_CULL_MODE_FRONT_BIT;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_deviceInfo->Device, m_deviceInfo->pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &m_skyboxPipeline));

		vkDestroyShaderModule(m_deviceInfo->Device, shaderstage[0].module, nullptr);
		vkDestroyShaderModule(m_deviceInfo->Device, shaderstage[1].module, nullptr);
	}

	void skybox::UpdateDescriptor()
	{
		descriptorInfo.imageInfo.imageLayout = m_imageLayout;
		descriptorInfo.imageInfo.imageView = m_imageView;
		descriptorInfo.imageInfo.sampler = m_sampler;
	}

	void skybox::CreateCube()
	{

		m_vertices = 

		{
		 // Back Face
		 {{ 1.0f,  1.0f, -1.0f}, {0, 0}, {0, 0, -1}, {-1, 0, 0}}, // Bottom-left
		 {{-1.0f,  1.0f, -1.0f}, {1, 0}, {0, 0, -1}, {-1, 0, 0}}, // Bottom-right
		 {{ 1.0f, -1.0f, -1.0f}, {0, 1}, {0, 0, -1}, {-1, 0, 0}}, // Top-left
		 {{-1.0f, -1.0f, -1.0f}, {1, 1}, {0, 0, -1}, {-1, 0, 0}}, // Top-right

			// Front Face

		 {{-1.0f, 1.0f,   1.0f}, {0, 0}, {0, 0, 1}, {1, 0, 0}}, // Bottom-left
		 {{ 1.0f, 1.0f,   1.0f}, {1, 0}, {0, 0, 1}, {1, 0, 0}}, // Bottom-right
		 {{-1.0f, -1.0f,  1.0f}, {0, 1}, {0, 0, 1}, {1, 0, 0}}, // Top-left
		 {{ 1.0f, -1.0f,  1.0f}, {1, 1}, {0, 0, 1}, {1, 0, 0}}, // Top-right

		 // Left Face
		 {{-1.0f,  1.0f, -1.0f}, {0, 0}, {-1, 0, 0}, {0, 0, 1}}, // Bottom-left
		 {{-1.0f,  1.0f,  1.0f}, {1, 0}, {-1, 0, 0}, {0, 0, 1}}, // Bottom-right
		 {{-1.0f, -1.0f, -1.0f}, {0, 1}, {-1, 0, 0}, {0, 0, 1}}, // Top-left
		 {{-1.0f, -1.0f,  1.0f}, {1, 1}, {-1, 0, 0}, {0, 0, 1}}, // Top-right

		 // Right Face
		 {{ 1.0f,  1.0f,  1.0f}, {0, 0}, {1, 0, 0}, {0, 0, -1}}, // Bottom-left
		 {{ 1.0f,  1.0f, -1.0f}, {1, 0}, {1, 0, 0}, {0, 0, -1}}, // Bottom-right
		 {{ 1.0f, -1.0f,  1.0f}, {0, 1}, {1, 0, 0}, {0, 0, -1}}, // Top-left
		 {{ 1.0f, -1.0f, -1.0f}, {1, 1}, {1, 0, 0}, {0, 0, -1}}, // Top-right

		 // Top Face
		 {{-1.0f,  -1.0f,  1.0f}, {0, 0}, {0, -1, 0}, {1, 0, 0}}, // Bottom-left
		 {{ 1.0f,  -1.0f,  1.0f}, {1, 0}, {0, -1, 0}, {1, 0, 0}}, // Bottom-right
		 {{-1.0f,  -1.0f, -1.0f}, {0, 1}, {0, -1, 0}, {1, 0, 0}}, // Top-left
		 {{ 1.0f,  -1.0f, -1.0f}, {1, 1}, {0, -1, 0}, {1, 0, 0}}, // Top-right

		 // Bottom Face
		 {{-1.0f, 1.0f, -1.0f}, {0, 0}, {0, 1, 0}, {1, 0, 0}}, // Bottom-left
		 {{ 1.0f, 1.0f, -1.0f}, {1, 0}, {0, 1, 0}, {1, 0, 0}}, // Bottom-right
		 {{-1.0f, 1.0f,  1.0f}, {0, 1}, {0, 1, 0}, {1, 0, 0}}, // Top-left
		 {{ 1.0f, 1.0f,  1.0f}, {1, 1}, {0, 1, 0}, {1, 0, 0}}, // Top-right


		};

		m_indices =  
		{
			4, 5, 6, 6, 5, 7,       // Back
			0, 1, 2, 2, 1, 3,       // Front
			8, 9, 10, 10, 9, 11,    // Left
			12, 13, 14, 14, 13, 15, // Right
			16, 17, 18, 18, 17, 19, // Top
			20, 21, 22, 22, 21, 23  // Bottom
		};

		for (int i = 0; i < m_vertices.size(); i++)
		{
			glm::vec3 pos = m_vertices[i].pos;
			m_vertices[i].col = glm::normalize(glm::vec4(glm::abs(pos), 1.0f));
		}

		//Making Buffer objects
		int buffer_size = (int)(sizeof(m_vertices[0]) * m_vertices.size());
		{
			Buffer stagingBuffer;

			stagingBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, m_vertices.data(), buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			m_vertexBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, nullptr, buffer_size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


			//Copy Host to Device
			initializer::copyBuffer(stagingBuffer.buffer, m_vertexBuffer.buffer, buffer_size, m_deviceInfo->commandPool, m_deviceInfo->Device, m_deviceInfo->GraphicsQueue);

			stagingBuffer.Destroy();

		}
		buffer_size = (int)(sizeof(unsigned int) * m_indices.size());
		{
			Buffer stagingBuffer;

			stagingBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, m_indices.data(), buffer_size,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			m_indexBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, nullptr, buffer_size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			//Copy Host to Device
			initializer::copyBuffer(stagingBuffer.buffer, m_indexBuffer.buffer, buffer_size, m_deviceInfo->commandPool, m_deviceInfo->Device, m_deviceInfo->GraphicsQueue);

			stagingBuffer.Destroy();
		}

	}


	void skybox::DrawSkybox(VkCommandBuffer cmdBuffer)
	{
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_deviceInfo->pipelineLayout, 1, 1, &descriptorInfo.descriptorSet, 0, nullptr);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_skyboxPipeline);

		VkBuffer vertexBuffer[] = { m_vertexBuffer.buffer };
		VkDeviceSize offset[] = { 0 };

		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffer, offset);
		vkCmdBindIndexBuffer(cmdBuffer, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmdBuffer, (uint32_t)m_indices.size(), 1, 0, 0, 0);
	}

	void skybox::Destroy()
	{
		m_vertexBuffer.Destroy();
		m_indexBuffer.Destroy();

		vkFreeDescriptorSets(m_deviceInfo->Device, m_deviceInfo->DescriptorPool, 1, &descriptorInfo.descriptorSet);
		vkDestroyPipeline(m_deviceInfo->Device, m_skyboxPipeline, nullptr);
		vkDestroyImage(m_deviceInfo->Device, m_image, nullptr);
		vkDestroyImageView(m_deviceInfo->Device, m_imageView, nullptr);
		vkDestroySampler(m_deviceInfo->Device, m_sampler, nullptr);
		vkFreeMemory(m_deviceInfo->Device, m_imageMemory, nullptr);
	}
}