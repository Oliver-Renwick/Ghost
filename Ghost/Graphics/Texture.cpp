#include "Texture.h"

/*TO-DO Write Command Buffer Structure and Finish the Transfering Host to Device Operation using that command Buffer*/

namespace graphics
{
	void Texture::updateDescriptor()
	{
		m_descriptorimageInfo.imageLayout = m_imageLayout;
		m_descriptorimageInfo.imageView = m_imageView;
		m_descriptorimageInfo.sampler = m_sampler;
	}

	void Texture::Destroy()
	{
		if (m_image) { vkDestroyImage(m_device, m_image, nullptr); }
		if (m_imageView) { vkDestroyImageView(m_device, m_imageView, nullptr); }
		if (m_sampler) { vkDestroySampler(m_device, m_sampler, nullptr); }
		if (m_deviceMemory) { vkFreeMemory(m_device, m_deviceMemory, nullptr); }

	}

	ktxResult Texture::loadKtxFile(std::string filename, ktxTexture** target)
	{
		ktxResult res = KTX_SUCCESS;

		if (!initializer::fileExist(filename))
		{
			throw std::runtime_error("Unable to load that Ktx File \n\nMake sure the assets submodule has been checked out and is up-to-date.");
		}

		res = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);

		return res;
	}

	void Texture2D::loadFromFile(std::string filename, VkFormat format, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout, bool forceLiner)
	{
		ktxTexture* m_ktxTexture;
		ktxResult res = loadKtxFile(filename, &m_ktxTexture);
		assert(res == KTX_SUCCESS);

		m_width = m_ktxTexture->baseWidth;
		m_height = m_ktxTexture->baseHeight;
		m_miplevels = m_ktxTexture->numLevels;;

		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(m_ktxTexture);
		ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(m_ktxTexture);

		// Get device properties for the requested texture format
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &properties);

		bool useStaging = !forceLiner;

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs{};

		VkCommandBuffer copyCmd{};  //

		if (useStaging)
		{
			VkBuffer stagingBuffer;
			VkDeviceMemory StagingMemory;

			VkBufferCreateInfo bufferInfo;
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = ktxTextureSize;

			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferInfo, nullptr, &stagingBuffer));

			vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memReqs);
			allocInfo.allocationSize = memReqs.size;
			allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_PhysicalDevice);

			VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &StagingMemory));
			VK_CHECK_RESULT(vkBindBufferMemory(m_device, stagingBuffer, StagingMemory, 0));

			//Transfer the data
			uint8_t* data;
			VK_CHECK_RESULT(vkMapMemory(m_device, StagingMemory, 0, memReqs.size, 0, (void**)&data));
			memcpy(data, ktxTextureData, ktxTextureSize);
			vkUnmapMemory(m_device, StagingMemory);

			//Buffer Copy Region for every miplevels

			std::vector<VkBufferImageCopy> bufferCopyRegions;

			for (uint32_t i = 0; i < m_miplevels; i++)
			{
				ktx_size_t offset;
				ktx_error_code_e result = ktxTexture_GetImageOffset(m_ktxTexture, i, 0, 0, &offset);
				//assert(result == KTX_SUCCESS);

				VkBufferImageCopy bufferCopyRegion{};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
				bufferCopyRegion.imageSubresource.mipLevel = i;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = std::max(1u, m_ktxTexture->baseWidth >> i);
				bufferCopyRegion.imageExtent.height = std::max(1u, m_ktxTexture->baseHeight >> i);
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;

				bufferCopyRegions.push_back(bufferCopyRegion);
			}

			//Optimal tiled image
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.mipLevels = m_miplevels;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.extent.width = m_width;
			imageInfo.extent.height = m_height;
			imageInfo.extent.depth = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = imageUsageFlags;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			if (!(imageInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
			{
				imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}
			VK_CHECK_RESULT(vkCreateImage(m_device, &imageInfo, nullptr, &m_image));

			vkGetImageMemoryRequirements(m_device, m_image, &memReqs);

			allocInfo.allocationSize = memReqs.size;
			allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_PhysicalDevice);

			VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &m_deviceMemory));
			VK_CHECK_RESULT(vkBindImageMemory(m_device, m_image, m_deviceMemory, 0));

			VkImageSubresourceRange subResourceRange{};
			subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subResourceRange.baseMipLevel = 0;
			subResourceRange.levelCount = m_miplevels;
			subResourceRange.layerCount = 1;

			initializer::setImageLayout(
				copyCmd,
				m_image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subResourceRange,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
			);

			//copy mipLevels from staging buffer
			vkCmdCopyBufferToImage(
				copyCmd,
				stagingBuffer,
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				static_cast<uint32_t>(bufferCopyRegions.size()),
				bufferCopyRegions.data()
			);

			// Change texture image layout to shader read after all mip levels have been copied
			this->m_imageLayout = imageLayout;
			initializer::setImageLayout(
				copyCmd,
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				imageLayout,
				subResourceRange,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
			);

			//End Command Buffer

			//m_renderer->endSingleCommand(copyCmd);

			vkDestroyBuffer(m_device, stagingBuffer, nullptr);
			vkFreeMemory(m_device, StagingMemory, nullptr);
		}

		else
		{
			// Prefer using optimal tiling, as linear tiling 
			// may support only a small set of features 
			// depending on implementation (e.g. no mip maps, only one layer, etc.)

			// Check if this support is supported for linear tiling
			assert(properties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

			VkImage mappableImage;
			VkDeviceMemory mappableMemory;

			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.extent.width = m_width;
			imageInfo.extent.height = m_height;
			imageInfo.extent.depth = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
			imageInfo.usage = imageUsageFlags;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;


			VK_CHECK_RESULT(vkCreateImage(m_device, &imageInfo, nullptr, &mappableImage));

			vkGetImageMemoryRequirements(m_device, mappableImage, &memReqs);

			allocInfo.allocationSize = memReqs.size;
			allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_PhysicalDevice);

			VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &mappableMemory));

			VK_CHECK_RESULT(vkBindImageMemory(m_device, mappableImage, mappableMemory, 0));

			VkImageSubresource subRes{};
			subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subRes.mipLevel = 0;

			VkSubresourceLayout subLayout{};
			void* data;

			vkGetImageSubresourceLayout(m_device, mappableImage, &subRes, &subLayout);

			VK_CHECK_RESULT(vkMapMemory(m_device, mappableMemory, 0, memReqs.size, 0, &data));

			memcpy(data, ktxTextureData, memReqs.size);

			vkUnmapMemory(m_device, mappableMemory);

			m_image = mappableImage;
			m_deviceMemory = mappableMemory;
			this->m_imageLayout = imageLayout;

			initializer::setImageLayout(copyCmd, m_image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);


			//m_renderer->endSingleCommand(copyCmd);
		}

		ktxTexture_Destroy(m_ktxTexture);


		//Creating Sampler 
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		// Max level-of-detail should match mip level count

		samplerInfo.maxLod = (useStaging) ? (float)m_miplevels : 0.0f;
		//samplerInfo.maxAnisotropy = m_renderer->device_properties.limits.maxSamplerAnisotropy;
		//samplerInfo.anisotropyEnable = m_renderer->device_features.samplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VK_CHECK_RESULT(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler));

		// Create image view
		// Textures are not directly accessed by the shaders and
		// are abstracted by image views containing additional
		// information and sub resource ranges
		VkImageViewCreateInfo imageviewCI{};
		imageviewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageviewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageviewCI.format = format;
		imageviewCI.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		imageviewCI.image = m_image;
		imageviewCI.subresourceRange.levelCount = (useStaging) ? m_miplevels : 1;

		VK_CHECK_RESULT(vkCreateImageView(m_device, &imageviewCI, nullptr, &m_imageView));

		updateDescriptor();
	}


	void Texture2D::FromBuffer(
		void* buffer,
		VkDeviceSize       bufferSize,
		VkFormat           format,
		uint32_t           texWidth,
		uint32_t           texHeight,
		VkFilter           filter,
		VkImageUsageFlags  imageUsageFlags,
		VkImageLayout      imageLayout
	)

	{
		assert(buffer);

		this->m_height = texHeight;
		this->m_width = texWidth;
		this->m_miplevels = 1;

		VkCommandBuffer copycmd{};

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.size = bufferSize;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferInfo, nullptr, &stagingBuffer));

		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo allocInfo{};

		vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memReqs);

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_PhysicalDevice);

		VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(m_device, stagingBuffer, stagingMemory, 0));

		uint8_t* data;
		vkMapMemory(m_device, stagingMemory, 0, memReqs.size, 0, (void**)&data);
		memcpy(data, buffer, bufferSize);
		vkUnmapMemory(m_device, stagingMemory);

		VkBufferImageCopy bufferCopyRegion{};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageExtent.width = m_width;
		bufferCopyRegion.imageExtent.height = m_height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.mipLevels = m_miplevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.extent.width = m_width;
		imageInfo.extent.height = m_height;
		imageInfo.extent.depth = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = imageUsageFlags;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (!(imageInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		VK_CHECK_RESULT(vkCreateImage(m_device, &imageInfo, nullptr, &m_image));

		vkGetImageMemoryRequirements(m_device, m_image, &memReqs);

		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_PhysicalDevice);
		VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &m_deviceMemory));
		VK_CHECK_RESULT(vkBindImageMemory(m_device, m_image, m_deviceMemory, 0));

		VkImageSubresourceRange subResourceRange{};
		subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResourceRange.baseMipLevel = 0;
		subResourceRange.levelCount = m_miplevels;
		subResourceRange.layerCount = 1;

		initializer::setImageLayout(
			copycmd,
			m_image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subResourceRange,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);

		vkCmdCopyBufferToImage(
			copycmd,
			stagingBuffer,
			m_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
		);


		this->m_imageLayout = imageLayout;

		initializer::setImageLayout(
			copycmd,
			m_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			imageLayout,
			subResourceRange,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);

		//m_renderer->endSingleCommand(copycmd);

		vkDestroyBuffer(m_device, stagingBuffer, nullptr);
		vkFreeMemory(m_device, stagingMemory, nullptr);


		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = filter;
		samplerInfo.minFilter = filter;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;

		VK_CHECK_RESULT(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler));

		//Create Image View

		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.format = format;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.image = m_image;
		imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0 , 1, 0 , 1 };
		imageViewInfo.subresourceRange.levelCount = 1;
		VK_CHECK_RESULT(vkCreateImageView(m_device, &imageViewInfo, nullptr, &m_imageView));

		updateDescriptor();
	}
}