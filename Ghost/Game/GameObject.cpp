#include "GameObject.h"

namespace game
{
	void GameObject::loadGameObject(std::string& gamePath, graphics::DeviceInfo& deviceInfo) 
	{
		modelResource.m_deviceInfo = deviceInfo;
		bool loaded = modelResource.load_Model(gamePath);
	}

	void GameObject::setPosition(glm::vec3& pos) { 
		translation = pos; 
	}

	void GameObject::setRotation(glm::vec3& rot) { 
		rotation = rot; 
	}

	void GameObject::setScale(glm::vec3& scale) { 
		this->scale = scale; 
	}
}