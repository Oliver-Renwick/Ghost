#pragma once
#include "Math/Vector.h"
#include "Math/Mat.h"
#include "Math/Quat.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace game
{
	struct Camera
	{
		glm::mat4 proj = glm::mat4(0.0f);
		glm::mat4 view = glm::mat4(0.0f);
		glm::vec3 pos = glm::vec3(0.0f);
		glm::vec3 rot = glm::vec3(0.0f);

		float yaw = glm::radians(0.0f);
		float pitch = -0.45;
		float rotationSpeed = 0.01f;
		float camRadius = 48.0f;

		void lookat(glm::vec3 to, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));
		void SetPerspective(float fov, float aspectratio, float _near, float _far);
		void Rotate(float x, float y);
		void UpdateOrbitCamera(const Vec3& objPos);

	};
}
