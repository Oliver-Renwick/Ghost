#include "Camera.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace game
{


	void Camera::lookat(glm::vec3 target, glm::vec3 up)
	{
		glm::vec3 cam_forward = glm::normalize(target - pos);
		glm::vec3 cam_right = glm::normalize(glm::cross(cam_forward, up));
		glm::vec3 cam_Up = glm::normalize(glm::cross(cam_right, cam_forward));

		//Row 1
		view[0][0] = cam_right.x;
		view[1][0] = cam_right.y;
		view[2][0] = cam_right.z;

		//Row 2
		view[0][1] = cam_Up.x;
		view[1][1] = cam_Up.y;
		view[2][1] = cam_Up.z;

		//Row 3
		view[0][2] = -cam_forward.x;
		view[1][2] = -cam_forward.y;
		view[2][2] = -cam_forward.z;

		view[3][0] = -dot(cam_right, pos);
		view[3][1] = -dot(cam_Up, pos);
		view[3][2] = dot(cam_forward, pos);
		view[3][3] = 1.0f;

	}

	void Camera::SetPerspective(float fov, float aspectratio, float near, float far)
	{
		//proj = glm::perspective(fov, aspectratio, near, far);
		//ToDo learn about right hand and left hand order in perspective pprojection
		assert(std::abs(aspectratio - std::numeric_limits<float>::epsilon()) > 0.0f);
		
		const float tanHalfFov = tan(fov / 2.0f);
		
		proj[0][0] = 1.0f / (aspectratio * tanHalfFov);
		proj[1][1] = 1.0f / tanHalfFov;
		proj[2][2] = - (far + near) / (far - near);
		proj[2][3] = - 1.0f;
		proj[3][2] = -(2.0f * far * near) / (far - near);

	}


	void Camera::UpdateOrbitCamera(const Vec3& objPos)
	{
		float x = objPos.x + camRadius * sin(yaw) * cos(pitch);
		float y = objPos.y + camRadius * sin(pitch);
		float z = objPos.z + camRadius * cos(yaw) * cos(pitch);


		pos = glm::vec3(x,y,z);

		lookat(glm::vec3(objPos.x, -objPos.y, objPos.z));

	}


	void Camera::Rotate(float deltax, float deltay)
	{
		yaw -= deltax * rotationSpeed;
		pitch -= deltay * rotationSpeed;

		if (pitch < glm::radians(-89.0f))
		{
			pitch = glm::radians(-89.0f);
		}
		else if (pitch > glm::radians(3.0f))
		{
			pitch = glm::radians(3.0f);
		}
	}
}