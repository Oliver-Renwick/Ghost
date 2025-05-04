#include "Material.h"

namespace graphics
{
	void MaterialManager::InitMaterials(DeviceInfo* deviceInfo)
	{
		if (deviceInfo) { m_deviceInfo = deviceInfo; }
		else { throw std::runtime_error("Give Valid Device Info"); }
		m_format = VK_FORMAT_R8G8B8A8_SRGB;
		//CreateImage();
	}

	void MaterialManager::CreateImage()
	{
		if (m_paths.empty()) { 
			std::cerr << "There is no Material paths to process" << std::endl; 
			return;
		}

		for (int i = 0; i < m_paths.size(); i++)
		{
			Texture texture{};
			texture.load_Texture(m_paths[i], m_deviceInfo,m_deviceInfo->commandPool, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, m_format, false);
			m_textures.push_back(texture);
		}

		m_size = m_paths.size();
	}

	void MaterialManager::UpdateMaterial(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout)
	{
		m_size = m_textures.size();
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &m_deviceInfo->bindless.update_after_bind_descriptorSet, 0, nullptr);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, &m_deviceInfo->sampler_descriptors.descriptorSet, 0, nullptr);

		uint32_t descriptorOffset{};

		for (size_t i = 0; i < m_size; i++)
		{
			VkDescriptorImageInfo imageInfo = initializer::descriptor_image_info(VK_NULL_HANDLE, m_textures[i].m_imageView, m_textures[i].m_imageLayout);
			VkWriteDescriptorSet write = initializer::writeDescriptorSet(m_deviceInfo->bindless.update_after_bind_descriptorSet, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, &imageInfo);

			write.dstArrayElement = descriptorOffset;
			descriptorOffset = (descriptorOffset + 1) % 2048; // change it when you change NumDescriptorsStreaming in bindless descripor function
			vkUpdateDescriptorSets(m_deviceInfo->Device, 1, &write, 0, nullptr);

			//Let each object has the index val assigned to them and be accesed when we drawing them in order
		}
	}

	void MaterialManager::Destroy()
	{
		m_paths.clear();
		for (int i = 0; i < m_textures.size(); i++)
		{
			m_textures[i].destroy();
		}
	}
}