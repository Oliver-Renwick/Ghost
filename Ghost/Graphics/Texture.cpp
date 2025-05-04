#include "Texture.h"

namespace graphics
{
	void Texture::load_Texture(const std::string& filename, DeviceInfo* deviceInfo, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer,
    VkSemaphore transfer_semaphore, VkFence fence ,const VkFormat& format, bool sampler , bool linearTiling)
	{
		assert(deviceInfo != nullptr);

		m_deviceInfo = deviceInfo;


		ktxTexture* ktx_Texture;
		KTX_error_code result;

		result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_Texture);

		assert(result == KTX_SUCCESS);

		m_width = ktx_Texture->baseWidth;
		m_height = ktx_Texture->baseHeight;
		ktx_uint8_t* ktx_imageData = ktxTexture_GetData(ktx_Texture);
		ktx_size_t ktx_TextureSize = ktxTexture_GetImageSize(ktx_Texture, 0);

		//Mipmap Generation
		m_miplevels = static_cast<uint32_t>(floor(log2(std::max(m_width, m_height))) + 1);

		VkFormatProperties formatProperties{};
		vkGetPhysicalDeviceFormatProperties(m_deviceInfo->PhysicalDevice, format, &formatProperties);

		assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);
		assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

		// Staging is used for copying the texture data to device local
		VkBool32 use_staging = true;

		if (linearTiling)
		{
			std::cout << "Used Linear Tiling" << std::endl;
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(m_deviceInfo->PhysicalDevice, format, &formatProperties);

			use_staging = !(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
		}

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

		VkMemoryRequirements memoryReq{};



		if (use_staging)
		{
			// Copy data to an optimal tiled image
			// This loads the texture data into a host local buffer that is copied to the optimal tiled image on the device

			// Create a host-visible staging buffer that contains the raw image data
			// This buffer will be the data source for copying texture data to the optimal tiled image on the device

			VkBuffer	   stagingBuffer;
			VkDeviceMemory stagingMemory;

			VkBufferCreateInfo bufferCreateInfo{};

			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.size = ktx_TextureSize;
			VK_CHECK_RESULT(vkCreateBuffer(m_deviceInfo->Device, &bufferCreateInfo, nullptr, &stagingBuffer));

			vkGetBufferMemoryRequirements(m_deviceInfo->Device, stagingBuffer, &memoryReq);

			allocInfo.allocationSize = memoryReq.size;
			allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_deviceInfo->PhysicalDevice);
			
			VK_CHECK_RESULT(vkAllocateMemory(m_deviceInfo->Device, &allocInfo, nullptr, &stagingMemory));
			VK_CHECK_RESULT(vkBindBufferMemory(m_deviceInfo->Device, stagingBuffer, stagingMemory, 0));


			//Copy Texture data into staging buffer
			uint8_t *data;
			VK_CHECK_RESULT(vkMapMemory(m_deviceInfo->Device, stagingMemory, 0, memoryReq.size, 0, (void**)&data));
			memcpy(data, ktx_imageData, ktx_TextureSize);
			vkUnmapMemory(m_deviceInfo->Device, stagingMemory);

		

			//Create Optimal Timled image
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = format;
			imageInfo.mipLevels = m_miplevels;
			imageInfo.arrayLayers = 1;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//set initial layout to undefined
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.extent = { m_width, m_height, 1 };
			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

			VK_CHECK_RESULT(vkCreateImage(m_deviceInfo->Device, &imageInfo, nullptr, &m_image));

			vkGetImageMemoryRequirements(m_deviceInfo->Device, m_image, &memoryReq);
			allocInfo.allocationSize = memoryReq.size;
			allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_deviceInfo->PhysicalDevice);
			VK_CHECK_RESULT(vkAllocateMemory(m_deviceInfo->Device, &allocInfo, nullptr, &m_imageMemory));
			VK_CHECK_RESULT(vkBindImageMemory(m_deviceInfo->Device, m_image, m_imageMemory, 0));

			//Begin Command Buffer

			//VkCommandBuffer copyCmd;
			//
			//VkCommandBufferAllocateInfo cmdAllocInfo{};
			//cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			//cmdAllocInfo.commandBufferCount = 1;
			//cmdAllocInfo.commandPool = m_deviceInfo->commandPool;
			//cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			//
			//VK_CHECK_RESULT(vkAllocateCommandBuffers(m_deviceInfo->Device, &cmdAllocInfo, &copyCmd));

			VkCommandBufferBeginInfo cmdBeginInfo{};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkResetCommandBuffer(cmdBuffer, 0);
			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo));

			// Image memory barriers for the texture image

			// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
			VkImageSubresourceRange subResource_range{};
			subResource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			// Start at first mip level
			subResource_range.baseMipLevel = 0;
			// We will transition on all mip levels
			subResource_range.levelCount = 1;
			// The 2D texture only has one layer
			subResource_range.layerCount = 1;



			// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
			VkImageMemoryBarrier image_memory_barrier{};
			image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			image_memory_barrier.image = m_image;
			image_memory_barrier.subresourceRange = subResource_range;
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			vkCmdPipelineBarrier(cmdBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &image_memory_barrier
			);

			// Setup buffer copy regions for each mip level
		
			VkBufferImageCopy bufferCopyRegion{};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = 0;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = m_width;
			bufferCopyRegion.imageExtent.height = m_height;
			bufferCopyRegion.imageExtent.depth = 1;



			// Copy mip levels from staging buffer
			vkCmdCopyBufferToImage(cmdBuffer,
				stagingBuffer,
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&bufferCopyRegion
			);

			// Transition first mip level to transfer source for read during blit
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

			
			vkCmdPipelineBarrier(
				cmdBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &image_memory_barrier
			);

			// Store current layout for later reuse
			m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			vkEndCommandBuffer(cmdBuffer);
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;
			VkPipelineStageFlags waitFlags[]{ VK_PIPELINE_STAGE_TRANSFER_BIT };
			VkSemaphore waitSemaphore[] = { transfer_semaphore };
			submitInfo.pWaitSemaphores = waitSemaphore;
			submitInfo.pWaitDstStageMask = waitFlags;

			vkQueueSubmit(m_deviceInfo->transferGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(m_deviceInfo->transferGraphicsQueue);
			//vkFreeCommandBuffers(m_deviceInfo->Device, cmdPool, 1, &cmdBuffer);

			vkDestroyBuffer(m_deviceInfo->Device, stagingBuffer, nullptr);
			vkFreeMemory(m_deviceInfo->Device, stagingMemory, nullptr);

			// Generate the mip chain
			// ---------------------------------------------------------------
			// We copy down the whole mip chain doing a blit from mip-1 to mip
			// An alternative way would be to always blit from the first mip level and sample that one down
			//VkCommandBuffer blitCmd{};
			//VkCommandBufferAllocateInfo blitcmdAllocInfo{};
			//blitcmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			//blitcmdAllocInfo.commandBufferCount = 1;
			//blitcmdAllocInfo.commandPool = m_deviceInfo->commandPool;
			//blitcmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			//
			//VK_CHECK_RESULT(vkAllocateCommandBuffers(m_deviceInfo->Device, &blitcmdAllocInfo, &blitCmd));

			VkCommandBufferBeginInfo blitcmdBeginInfo{};
			blitcmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			blitcmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkResetCommandBuffer(cmdBuffer, 0);
			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo));

			for (uint32_t i = 1; i < m_miplevels; i++)
			{
				VkImageBlit imageBlit{};

				//Source
				imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBlit.srcSubresource.layerCount = 1;
				imageBlit.srcSubresource.mipLevel = i - 1;
				imageBlit.srcOffsets[1].x = int32_t(m_width >> (i - 1));
				imageBlit.srcOffsets[1].y = int32_t(m_height >> (i - 1));
				imageBlit.srcOffsets[1].z = 1;

				//Destination
				imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBlit.dstSubresource.layerCount = 1;
				imageBlit.dstSubresource.mipLevel = i;
				imageBlit.dstOffsets[1].x = int32_t(m_width >> i);
				imageBlit.dstOffsets[1].y = int32_t(m_height >> i);
				imageBlit.dstOffsets[1].z = 1;


				VkImageSubresourceRange mipSubRange = {};
				mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				mipSubRange.baseMipLevel = i;
				mipSubRange.levelCount = 1;
				mipSubRange.layerCount = 1;

				// Prepare current mip level as image blit destination
				VkImageMemoryBarrier blit_image_memory_barrier{};
				blit_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				blit_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				blit_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

				blit_image_memory_barrier.image = m_image;
				blit_image_memory_barrier.subresourceRange = mipSubRange;
				blit_image_memory_barrier.srcAccessMask = 0;
				blit_image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				blit_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				blit_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

				vkCmdPipelineBarrier(
					cmdBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &blit_image_memory_barrier
				);


				vkCmdBlitImage(
					cmdBuffer,
					m_image,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					m_image,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&imageBlit,
					VK_FILTER_LINEAR);

				// Prepare current mip level as image blit source for next level
				blit_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				blit_image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				blit_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				blit_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

				vkCmdPipelineBarrier(
					cmdBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &blit_image_memory_barrier
				);
			}

			// After the loop,  all mip layers are in TRANSFER_SRC layout, so transition all to SHADER_READ
			subResource_range.levelCount = m_miplevels;

			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			vkCmdPipelineBarrier(
				cmdBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &image_memory_barrier
			);


			vkEndCommandBuffer(cmdBuffer);
			VkSubmitInfo blitsubmitInfo{};
			blitsubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			blitsubmitInfo.commandBufferCount = 1;
			blitsubmitInfo.pCommandBuffers = &cmdBuffer;
			blitsubmitInfo.pWaitSemaphores = waitSemaphore;
			blitsubmitInfo.pWaitDstStageMask = waitFlags;

			vkQueueSubmit(m_deviceInfo->transferGraphicsQueue, 1, &blitsubmitInfo, fence);
			vkQueueWaitIdle(m_deviceInfo->transferGraphicsQueue);
			//vkFreeCommandBuffers(m_deviceInfo->Device, cmdPool, 1, &cmdBuffer);

		}

		else
		{

			VkImage		   mappableImage;
			VkDeviceMemory mappableMemory;

			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = format;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
			imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			imageInfo.extent = { m_width, m_height, 1 };

			VK_CHECK_RESULT(vkCreateImage(m_deviceInfo->Device, &imageInfo, nullptr, &mappableImage));

			vkGetImageMemoryRequirements(m_deviceInfo->Device, mappableImage, &memoryReq);
			allocInfo.allocationSize = memoryReq.size;
			allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_deviceInfo->PhysicalDevice);
			VK_CHECK_RESULT(vkAllocateMemory(m_deviceInfo->Device, &allocInfo, nullptr, &mappableMemory));
			VK_CHECK_RESULT(vkBindImageMemory(m_deviceInfo->Device, mappableImage, mappableMemory, 0));

			void* data;
			ktx_size_t ktx_image_size = ktxTexture_GetImageSize(ktx_Texture, 0);
			VK_CHECK_RESULT(vkMapMemory(m_deviceInfo->Device, mappableMemory, 0, memoryReq.size, 0, &data));
			memcpy(data, ktx_imageData, ktx_image_size);
			vkUnmapMemory(m_deviceInfo->Device, mappableMemory);


			m_image = mappableImage;
			m_imageMemory = mappableMemory;
			m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


			//VkCommandBuffer copyCmd;
			//
			//VkCommandBufferAllocateInfo cmdAllocInfo{};
			//cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			//cmdAllocInfo.commandBufferCount = 1;
			//cmdAllocInfo.commandPool = m_deviceInfo->commandPool;
			//cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			//
			//VK_CHECK_RESULT(vkAllocateCommandBuffers(m_deviceInfo->Device, &cmdAllocInfo, &copyCmd));

			VkCommandBufferBeginInfo cmdBeginInfo{};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkResetCommandBuffer(cmdBuffer, 0);
			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo));


			VkImageSubresourceRange subresourcerange{};
			subresourcerange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourcerange.baseMipLevel = 0;
			subresourcerange.levelCount = 1;
			subresourcerange.layerCount = 1;


			VkImageMemoryBarrier image_memory_barrier{};
			image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			image_memory_barrier.image = m_image;
			image_memory_barrier.subresourceRange = subresourcerange;
			image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


			vkCmdPipelineBarrier(
				cmdBuffer,
				VK_PIPELINE_STAGE_HOST_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &image_memory_barrier
			);

			vkEndCommandBuffer(cmdBuffer);
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;
			VkSemaphore waitSemaphore[] = { transfer_semaphore };
			VkPipelineStageFlags waitFlags[]{ VK_PIPELINE_STAGE_TRANSFER_BIT };
			submitInfo.pWaitSemaphores = waitSemaphore;
			submitInfo.pWaitDstStageMask = waitFlags;

			vkQueueSubmit(m_deviceInfo->transferGraphicsQueue, 1, &submitInfo, fence);
			vkQueueWaitIdle(m_deviceInfo->transferGraphicsQueue);
		//	vkFreeCommandBuffers(m_deviceInfo->Device, cmdPool, 1, &cmdBuffer);

		}

		ktxTexture_Destroy(ktx_Texture);

		// Calculate valid filter and mipmap modes
		if (sampler)
		{
			VkFilter filter = VK_FILTER_LINEAR;
			VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			make_filters_valid(m_deviceInfo->PhysicalDevice, format, &filter, &mipmap_mode);

			// Create a texture sampler
			// In Vulkan textures are accessed by samplers
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.minFilter = filter;
			samplerInfo.magFilter = filter;
			samplerInfo.mipmapMode = mipmap_mode;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = (use_staging) ? static_cast<float>(m_miplevels) : 0.0f;

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

			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			VK_CHECK_RESULT(vkCreateSampler(m_deviceInfo->Device, &samplerInfo, nullptr, &m_sampler));
		}

		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.levelCount = (use_staging) ? m_miplevels : 1;

		imageViewCreateInfo.image = m_image;

		VK_CHECK_RESULT(vkCreateImageView(m_deviceInfo->Device, &imageViewCreateInfo, nullptr, &m_imageView));



	}

	void Texture::destroy()
	{
		
		vkDestroyImageView(m_deviceInfo->Device, m_imageView, nullptr);
		vkDestroyImage(m_deviceInfo->Device, m_image, nullptr);
		vkFreeMemory(m_deviceInfo->Device, m_imageMemory, nullptr);
		if (m_sampler != VK_NULL_HANDLE) { vkDestroySampler(m_deviceInfo->Device, m_sampler, nullptr); }
	}
}