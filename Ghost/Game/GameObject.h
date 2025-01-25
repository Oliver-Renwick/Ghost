#pragma once
#include "Graphics/Vulkan_GltfResource.h"
#include "glm/glm.hpp"

static int IDX = 0;

namespace game
{

	struct GameObject
	{
		graphics::GltfModel modelResource;

		glm::vec3 translation = glm::vec3();
		glm::vec3 rotation = glm::vec3();
		glm::vec3 scale = glm::vec3();
		glm::mat4 modelMatrix = glm::mat4();
		uint32_t id = 0;


		void loadGameObject(std::string& gamePath, graphics::DeviceInfo& deviceInfo);
		void setPosition(glm::vec3& pos);
		void setRotation(glm::vec3& rot);
		void setScale(glm::vec3& scale);

		void SetId() { 
			id = IDX;
			IDX++; 
		}


	
	};
}