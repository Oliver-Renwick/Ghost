#include "Render.h"

// Renderer can take care of render pass and frame buffer .
/*WE may have from here*/

namespace graphics
{
	void Renderer::Init(DeviceInfo* deviceInfo, OffscreenData* offScreenData, skybox* skyBox, MaterialManager* material)
	{
		m_deviceInfo = deviceInfo;
		m_offScreenInfo = offScreenData;
		m_skyBox = skyBox;
		m_material = material;
		CreateColorResource();
		CreateDepthResource();
		CreateDefaultrenderPass_FrameBuffer();
		CreateOffScreenFrameBuffer();
	}

	void Renderer::PrepareOffscreenRenderPass()
	{
		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.format = m_offScreenInfo->offScreenDepthFormat;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthreference{};
		depthreference.attachment = 0;
		depthreference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		// Attachment will be used as depth/stencil during render pass

		VkSubpassDescription subPass{};
		subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subPass.colorAttachmentCount = 0;                                  //We dont need Color Attachment
		subPass.pDepthStencilAttachment = &depthreference;

		std::array<VkSubpassDependency, 2> dependencies{};

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderpassInfo{};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpassInfo.attachmentCount = 1;
		renderpassInfo.pAttachments = &attachmentDescription;
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subPass;
		renderpassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderpassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(m_deviceInfo->Device, &renderpassInfo, nullptr, &m_offScreenInfo->renderPass));
	}

	// Setup the offscreen framebuffer for rendering the scene from light's point-of-view to
	// The depth attachment of this framebuffer will then be used to sample from in the fragment shader of the shadowing pass
	void Renderer::CreateOffScreenFrameBuffer()
	{
		//Depth Image for Shadow mapping
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.height = m_offScreenInfo->height;
		imageInfo.extent.width = m_offScreenInfo->width;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.format = m_offScreenInfo->offScreenDepthFormat;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		VK_CHECK_RESULT(vkCreateImage(m_deviceInfo->Device, &imageInfo, nullptr, &m_offScreenInfo->depthImage));

		VkMemoryRequirements memReqs{};
		VkMemoryAllocateInfo allocInfo{};

		vkGetImageMemoryRequirements(m_deviceInfo->Device, m_offScreenInfo->depthImage, &memReqs);

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = initializer::findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_deviceInfo->PhysicalDevice);

		VK_CHECK_RESULT(vkAllocateMemory(m_deviceInfo->Device, &allocInfo, nullptr, &m_offScreenInfo->depthMemory));
		VK_CHECK_RESULT(vkBindImageMemory(m_deviceInfo->Device, m_offScreenInfo->depthImage, m_offScreenInfo->depthMemory, 0));


		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.format = m_offScreenInfo->offScreenDepthFormat;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.image = m_offScreenInfo->depthImage;

		VK_CHECK_RESULT(vkCreateImageView(m_deviceInfo->Device, &viewCreateInfo, nullptr, &m_offScreenInfo->depthView));

		VkFilter shadowmap_Filter = formatisFilterable(m_deviceInfo->PhysicalDevice, m_offScreenInfo->offScreenDepthFormat, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = shadowmap_Filter;
		samplerInfo.minFilter = shadowmap_Filter;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(m_deviceInfo->Device, &samplerInfo, nullptr, &m_offScreenInfo->depthsampler));

		PrepareOffscreenRenderPass();

		//Create FrameBuffer
		VkFramebufferCreateInfo frameBufferInfo{};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = m_offScreenInfo->renderPass;
		frameBufferInfo.attachmentCount = 1;
		frameBufferInfo.pAttachments = &m_offScreenInfo->depthView;
		frameBufferInfo.width = m_offScreenInfo->width;
		frameBufferInfo.height = m_offScreenInfo->height;
		frameBufferInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(m_deviceInfo->Device, &frameBufferInfo, nullptr, &m_offScreenInfo->frameBuffer));
	}

	void Renderer::DrawFrame(std::vector<RenderModel>& renderModels)
	{
		m_deviceInfo->current_frame = currentFrame;
		vkWaitForFences(m_deviceInfo->Device, 1, &m_deviceInfo->inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

		VkResult res{};
		uint32_t imageIndex;

		res = vkAcquireNextImageKHR(m_deviceInfo->Device, m_deviceInfo->_swapChain, UINT64_MAX, m_deviceInfo->imageAvailableSemaphore[currentFrame],
			VK_NULL_HANDLE, &imageIndex);
		if (res == VK_ERROR_OUT_OF_DATE_KHR)
		{
			/*TO Swapchain Recreation*/
			std::cout << "Out 0f Date" << std::endl;
			Re_CreateSwapchain();
			return;
		}
		else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(m_deviceInfo->Device, 1, &m_deviceInfo->inFlightFence[currentFrame]);

		vkResetCommandBuffer(m_deviceInfo->_CommandBuffers[currentFrame], 0);

		RecordCommandBuffer(m_deviceInfo->_CommandBuffers[currentFrame], m_deviceInfo->pipelineLayout, imageIndex, renderModels);

		//Submitting Command Buffer
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		
		VkSemaphore waitSemaphore[] = { m_deviceInfo->imageAvailableSemaphore[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.pWaitSemaphores = waitSemaphore;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_deviceInfo->_CommandBuffers[currentFrame];

		VkSemaphore signalSemaphore[] = { m_deviceInfo->renderFinishedSemaphore[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphore;

		VK_CHECK_RESULT(vkQueueSubmit(m_deviceInfo->GraphicsQueue, 1, &submitInfo, m_deviceInfo->inFlightFence[currentFrame]));

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphore;

		VkSwapchainKHR swapchains[] = { m_deviceInfo->_swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;

		res = vkQueuePresentKHR(m_deviceInfo->GraphicsQueue, &presentInfo);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
		{
			Re_CreateSwapchain();
		}
		else if (res != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to Present Swapchain Images");
		}

		currentFrame = (currentFrame + 1) % m_deviceInfo->MAX_FRAME_IN_FLIGHT;
	}

	void Renderer::RecordCommandBuffer(VkCommandBuffer cmdBuffer, VkPipelineLayout layout, uint32_t imageIndex, std::vector<RenderModel>& renderModels)
	{
		std::array<VkClearValue, 2> clearValue{};
		VkViewport viewPort{};
		VkRect2D scissors{};

		VkCommandBufferBeginInfo cmdbeginInfo{};
		cmdbeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdbeginInfo.flags = 0;
		cmdbeginInfo.pInheritanceInfo = nullptr;

		VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdbeginInfo));

		/*
			First render pass: Generate shadow map by rendering the scene from light's POV
		*/

		{
			clearValue[0].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = m_offScreenInfo->renderPass;
			renderPassBeginInfo.framebuffer = m_offScreenInfo->frameBuffer;

			renderPassBeginInfo.renderArea.offset = { 0,0 };
			renderPassBeginInfo.renderArea.extent.width = m_offScreenInfo->width;
			renderPassBeginInfo.renderArea.extent.height = m_offScreenInfo->height;
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = clearValue.data();

			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			viewPort.height = (float)m_offScreenInfo->height;
			viewPort.width = (float)m_offScreenInfo->width;
			viewPort.minDepth = 0.0f;
			viewPort.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewPort);

			scissors.offset = { 0, 0 };
			scissors.extent.width = m_offScreenInfo->width;
			scissors.extent.height = m_offScreenInfo->height;
			vkCmdSetScissor(cmdBuffer, 0, 1, &scissors);

			// Set depth bias (aka "Polygon offset")
			// Required to avoid shadow mapping artifacts
			vkCmdSetDepthBias(
				cmdBuffer,
				m_offScreenInfo->depthBiasConstant,
				0.0f,
				m_offScreenInfo->depthBiasSlope
			);

			vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_deviceInfo->pipelineLayout, 0, 1, &m_offScreenInfo->descriptorSet,
				0, nullptr);


			vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_offScreenInfo->pipeline);

			for (int i = 0; i < renderModels.size(); i++)
			{
				
				if (renderModels[i].model)
				{
					renderModels[i].model->DrawIndexed(cmdBuffer, layout);
				}
				else if (renderModels[i].gltfModel)
				{
					renderModels[i].gltfModel->draw(cmdBuffer, layout);
				}
			}

			vkCmdEndRenderPass(cmdBuffer);
		}


		{
			clearValue[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValue[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = m_deviceInfo->renderPass;
			renderPassBeginInfo.framebuffer = m_deviceInfo->frameBuffers[imageIndex];

			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = m_deviceInfo->swapchainextent;


			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValue.size());
			renderPassBeginInfo.pClearValues = clearValue.data();

			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			viewPort.x = 0.0f;
			viewPort.y = 0.0f;
			viewPort.minDepth = 0.0f;
			viewPort.maxDepth = 1.0f;
			viewPort.width = static_cast<float>(m_deviceInfo->swapchainextent.width);
			viewPort.height = static_cast<float>(m_deviceInfo->swapchainextent.height);



			vkCmdSetViewport(cmdBuffer, 0, 1, &viewPort);

			scissors.offset = { 0, 0 };
			scissors.extent = m_deviceInfo->swapchainextent;

			vkCmdSetScissor(cmdBuffer, 0, 1, &scissors);

			if (m_deviceInfo->displayShadowMap)
			{
				vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_deviceInfo->pipelineLayout,
					0, 1, &m_offScreenInfo->debugDescriptor, 0, nullptr);
				vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_offScreenInfo->debugPipeline);
			}
			else
			{
				vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_deviceInfo->pipelineLayout,
					0, 1, &m_deviceInfo->descriptorSet, 0, nullptr);

				m_skyBox->DrawSkybox(cmdBuffer);

				vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_deviceInfo->pipeline);
				m_material->UpdateMaterial(cmdBuffer, layout);
			}
			
			for (int i = 0; i < renderModels.size(); i++)
			{
				if (renderModels[i].model)
				{
					renderModels[i].model->DrawIndexed(cmdBuffer, layout);
				}
				else if (renderModels[i].gltfModel)
				{
					renderModels[i].gltfModel->draw(cmdBuffer, layout);
				}
			}



			vkCmdEndRenderPass(cmdBuffer);

		}

		VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
	}

	void Renderer::Re_CreateSwapchain()
	{
		vkDeviceWaitIdle(m_deviceInfo->Device);
		//Window Size
		RECT rect{};
		if (GetClientRect(m_deviceInfo->window, &rect))
		{
			m_deviceInfo->height = rect.right - rect.left;
			m_deviceInfo->width = rect.bottom - rect.top;
		}

		/*To - Do : there is an error where if i have any one of the dimension of width or height to zero i get validation error
		* regarding  render pass extent should not be 0.0.
		*/

		//Clean Ups
		for (VkFramebuffer frameBuf : m_deviceInfo->frameBuffers)
		{
			vkDestroyFramebuffer(m_deviceInfo->Device, frameBuf, nullptr);
		}

		vkDestroyImageView(m_deviceInfo->Device, colorImageView, nullptr);
		vkFreeMemory(m_deviceInfo->Device, colorImageMemory, nullptr);
		vkDestroyImage(m_deviceInfo->Device, colorImage, nullptr);

		vkDestroyImageView(m_deviceInfo->Device, depthView, nullptr);
		vkFreeMemory(m_deviceInfo->Device, DepthImageMemory, nullptr);
		vkDestroyImage(m_deviceInfo->Device, depthImage, nullptr);
		for (auto& swapView : m_deviceInfo->swapchainImageView)
		{
			vkDestroyImageView(m_deviceInfo->Device, swapView, nullptr);
		}

		vkDestroySwapchainKHR(m_deviceInfo->Device, m_deviceInfo->_swapChain, nullptr);


		GPUDevice::InitSwapchain(m_deviceInfo);
		CreateColorResource();
		CreateDepthResource();
		//Frame Buffer
		m_deviceInfo->frameBuffers.resize(m_deviceInfo->swapchain_Images.size());

		for (int i = 0; i < m_deviceInfo->frameBuffers.size(); i++)
		{
			std::array<VkImageView, 3> attachments = { colorImageView, depthView, m_deviceInfo->swapchainImageView[i] };

			VkFramebufferCreateInfo frameBufferCI{};
			frameBufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCI.renderPass = m_deviceInfo->renderPass;
			frameBufferCI.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferCI.pAttachments = attachments.data();
			frameBufferCI.width = m_deviceInfo->swapchainextent.width;
			frameBufferCI.height = m_deviceInfo->swapchainextent.height;
			frameBufferCI.layers = 1;

			VK_CHECK_RESULT(vkCreateFramebuffer(m_deviceInfo->Device, &frameBufferCI, nullptr, &m_deviceInfo->frameBuffers[i]));
		}

	}

	void Renderer::CreateColorResource()
	{

		VkFormat colorFormat = m_deviceInfo->swapchainImageFormat;

		initializer::createImage(m_deviceInfo->swapchainextent.width, m_deviceInfo->swapchainextent.height, colorFormat, m_deviceInfo->msaaSamples, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, m_deviceInfo->Device,
			m_deviceInfo->PhysicalDevice);

		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = colorFormat;
		imageViewInfo.image = colorImage;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.subresourceRange.levelCount = 1;

		VK_CHECK_RESULT(vkCreateImageView(m_deviceInfo->Device, &imageViewInfo, nullptr, &colorImageView));
	}

	void Renderer::CreateDepthResource()
	{
		depthFormat = initializer::findSupportedFormat(m_deviceInfo->PhysicalDevice,
			{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		uint32_t d_width = m_deviceInfo->swapchainextent.width;
		uint32_t d_height = m_deviceInfo->swapchainextent.height;

		VkPipelineStageFlags sourceStage{};
		VkPipelineStageFlags destinationStage{};
		
		//Image Creation
		initializer::createImage(d_width, d_height, depthFormat, m_deviceInfo->msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, DepthImageMemory, m_deviceInfo->Device, m_deviceInfo->PhysicalDevice);

		//Image View
		VkImageViewCreateInfo imageViewCI{};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = depthFormat;
		imageViewCI.image = depthImage;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;
		imageViewCI.subresourceRange.levelCount = 1;
		VK_CHECK_RESULT(vkCreateImageView(m_deviceInfo->Device, &imageViewCI, nullptr, &depthView));

		// Transistioning Image Layout
		VkCommandBuffer cmdBuffer{};

		//Begin Command Buffer
		VkCommandBufferAllocateInfo cmdAllocInfo{};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.commandPool = m_deviceInfo->commandPool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_deviceInfo->Device, &cmdAllocInfo, &cmdBuffer));

		VkCommandBufferBeginInfo cmdBeginInfo{};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo));

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		barrier.image = depthImage;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		vkCmdPipelineBarrier(cmdBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		vkEndCommandBuffer(cmdBuffer);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit(m_deviceInfo->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_deviceInfo->GraphicsQueue);
		vkFreeCommandBuffers(m_deviceInfo->Device, m_deviceInfo->commandPool, 1, &cmdBuffer);


	}

	void Renderer::CreateDefaultrenderPass_FrameBuffer()
	{

		//RenderPass
		
		//Color Attachment
		VkAttachmentDescription colorAttachment{};
		colorAttachment.samples = m_deviceInfo->msaaSamples;
		colorAttachment.format = m_deviceInfo->swapchainImageFormat;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//Depth Attachment
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = m_deviceInfo->msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		//Color Resolve Attachment
		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = m_deviceInfo->swapchainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmetnResolveRef{};
		colorAttachmetnResolveRef.attachment = 2;
		colorAttachmetnResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmetnResolveRef;

		VkSubpassDependency subpassDependency{};
		subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass = 0;

		subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpassDependency.srcAccessMask = 0;
		subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &subpassDependency;

		VK_CHECK_RESULT(vkCreateRenderPass(m_deviceInfo->Device, &renderPassInfo, nullptr, &m_deviceInfo->renderPass));


		//FrameBuffer
		m_deviceInfo->frameBuffers.resize(m_deviceInfo->swapchain_Images.size());

		for (int i = 0; i < m_deviceInfo->frameBuffers.size(); i++)
		{
			std::array<VkImageView, 3> attachments = { colorImageView, depthView, m_deviceInfo->swapchainImageView[i] };

			VkFramebufferCreateInfo frameBufferCI{};
			frameBufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCI.renderPass = m_deviceInfo->renderPass;
			frameBufferCI.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferCI.pAttachments = attachments.data();
			frameBufferCI.width = m_deviceInfo->swapchainextent.width;
			frameBufferCI.height = m_deviceInfo->swapchainextent.height;
			frameBufferCI.layers = 1;

			VK_CHECK_RESULT(vkCreateFramebuffer(m_deviceInfo->Device, &frameBufferCI, nullptr, &m_deviceInfo->frameBuffers[i]));
		}

	}

	void Renderer::preparePipeline()
	{
		VkPipelineShaderStageCreateInfo vertex_stage = createshaderStage("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/scene_vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_deviceInfo->Device);
		VkPipelineShaderStageCreateInfo fragment_stage = createshaderStage("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/scene_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_deviceInfo->Device);
		
		VkPipelineShaderStageCreateInfo shaderstage[2];

		VkVertexInputBindingDescription vertexInputBindingDescription = vert::GetBindingDescription();
		std::array<VkVertexInputAttributeDescription, 5> attributeDescription = vert::GetAttributeDescription();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();


		
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = initializer::inputAssemblySate(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0);
		VkViewport viewportCI = initializer::viewPort(m_deviceInfo->width, m_deviceInfo->height, 0.0f, 1.0f);
		VkRect2D scissor = initializer::rect2D((uint32_t)m_deviceInfo->width, (uint32_t)m_deviceInfo->height, 0, 0);
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicstateCI = initializer::dynamicStateInfo(dynamicStates);
		VkPipelineViewportStateCreateInfo viewportstateCI = initializer::viewportStateInfo(1, 1, &viewportCI, &scissor);
		VkPipelineRasterizationStateCreateInfo rasterCI = initializer::razterizationState(VK_POLYGON_MODE_FILL, 
			VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE); // Change it if you got any errors regarding triangles
		VkPipelineMultisampleStateCreateInfo multisampleCI = initializer::MultiSampleStateInfo(m_deviceInfo->msaaSamples);
		VkPipelineColorBlendAttachmentState colorblendattachment = initializer::colorBlendAttachment(VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorblendstate = initializer::colorBlendInfo(1, &colorblendattachment);
		VkPipelineDepthStencilStateCreateInfo depthStencilstate = initializer::depthStencilInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderstage;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportstateCI;
		pipelineInfo.pRasterizationState = &rasterCI;
		pipelineInfo.pMultisampleState = &multisampleCI;
		pipelineInfo.pColorBlendState = &colorblendstate;
		pipelineInfo.pDynamicState = &dynamicstateCI;
		pipelineInfo.pDepthStencilState = &depthStencilstate;

		pipelineInfo.layout = m_deviceInfo->pipelineLayout;
		pipelineInfo.renderPass = m_deviceInfo->renderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		//Shadow Mapping Debug and Display
		rasterCI.cullMode = VK_CULL_MODE_NONE;
		shaderstage[0] = createshaderStage("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/quad_vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_deviceInfo->Device);
		shaderstage[1] = createshaderStage("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/quad_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_deviceInfo->Device);

		// Empty vertex input state
		VkPipelineVertexInputStateCreateInfo emptyInputState{};
		emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineInfo.pVertexInputState = &emptyInputState;


		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_deviceInfo->Device, m_deviceInfo->pipelineCache, 1, &pipelineInfo, nullptr, &m_offScreenInfo->debugPipeline));
		
		vkDestroyShaderModule(m_deviceInfo->Device, shaderstage[0].module, nullptr);
		vkDestroyShaderModule(m_deviceInfo->Device, shaderstage[1].module, nullptr);
		
		//Scene Rendering
		rasterCI.cullMode = VK_CULL_MODE_BACK_BIT;
		shaderstage[0] = vertex_stage;
		shaderstage[1] = fragment_stage;

		pipelineInfo.pVertexInputState = &vertexInputInfo;

		multisampleCI.rasterizationSamples = m_deviceInfo->msaaSamples;


		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_deviceInfo->Device, m_deviceInfo->pipelineCache, 1, &pipelineInfo, nullptr, &m_deviceInfo->pipeline));

		vkDestroyShaderModule(m_deviceInfo->Device, shaderstage[0].module, nullptr);
		vkDestroyShaderModule(m_deviceInfo->Device, shaderstage[1].module, nullptr);


		//OffScreen Renderer Pipeline
		VkPipelineShaderStageCreateInfo offscreenVertexShader = createshaderStage("C:/Users/ASUS/Desktop/Networking Ghost/Ghost/Asset/OffScreen_vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_deviceInfo->Device);

		shaderstage[0] = offscreenVertexShader;
		pipelineInfo.stageCount = 1;

		colorblendstate.attachmentCount = 0;
		depthStencilstate.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		rasterCI.cullMode = VK_CULL_MODE_NONE;
		rasterCI.depthBiasEnable = VK_TRUE;

		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		dynamicstateCI = initializer::dynamicStateInfo(dynamicStates);

		pipelineInfo.renderPass = m_offScreenInfo->renderPass;

		multisampleCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_deviceInfo->Device, m_deviceInfo->pipelineCache, 1, &pipelineInfo, nullptr, &m_offScreenInfo->pipeline));

		vkDestroyShaderModule(m_deviceInfo->Device, shaderstage[0].module, nullptr);


		m_skyBox->preparePipeline();
		   
	}

	void Renderer::Shutdown()
	{
		vkDestroyPipeline(m_deviceInfo->Device, m_offScreenInfo->pipeline, nullptr);
		vkDestroyRenderPass(m_deviceInfo->Device, m_offScreenInfo->renderPass, nullptr);
		vkDestroyFramebuffer(m_deviceInfo->Device, m_offScreenInfo->frameBuffer, nullptr);

		vkDestroySampler(m_deviceInfo->Device, m_offScreenInfo->depthsampler, nullptr);

		vkDestroyPipeline(m_deviceInfo->Device, m_offScreenInfo->debugPipeline, nullptr);

		vkDestroyPipeline(m_deviceInfo->Device, m_deviceInfo->pipeline, nullptr);

		for (VkFramebuffer frameBuf : m_deviceInfo->frameBuffers)
		{
			vkDestroyFramebuffer(m_deviceInfo->Device, frameBuf, nullptr);
		}

		vkDestroyRenderPass(m_deviceInfo->Device, m_deviceInfo->renderPass, nullptr);

		vkDestroyImageView(m_deviceInfo->Device, depthView, nullptr);
		vkFreeMemory(m_deviceInfo->Device, DepthImageMemory, nullptr);
		vkDestroyImage(m_deviceInfo->Device, depthImage, nullptr);

		vkDestroyImageView(m_deviceInfo->Device, colorImageView, nullptr);
		vkFreeMemory(m_deviceInfo->Device, colorImageMemory, nullptr);
		vkDestroyImage(m_deviceInfo->Device, colorImage, nullptr);

		vkDestroyImageView(m_deviceInfo->Device, m_offScreenInfo->depthView, nullptr);
		vkDestroyImage(m_deviceInfo->Device, m_offScreenInfo->depthImage, nullptr);
		vkFreeMemory(m_deviceInfo->Device, m_offScreenInfo->depthMemory, nullptr);
	}
}