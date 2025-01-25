#pragma once

#include "Math/Mat.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Physics/Shapes/Shapes.h"
#include "Buffer.h"
#include "Physics/Shapes/Sphere_Shape.h"
#include "Physics/Shapes/Box_Shape.h"
#include "Vulkan_Init.h"
#include "glm/glm.hpp"
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan.h>
#include <array>

namespace graphics
{

	struct vert
	{
		glm::vec3	pos;  
		glm::vec2	uv;   
		glm::vec3	norm; 
		glm::vec3	tang; 
		glm::vec4	col; 

		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription vertexInputBindingDescription{};
			vertexInputBindingDescription.binding = 0;
			vertexInputBindingDescription.stride = sizeof(vert);
			vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return vertexInputBindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 5> GetAttributeDescription()
		{
			std::array<VkVertexInputAttributeDescription, 5> attributeDescription{};

			attributeDescription[0].binding = 0;
			attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription[0].location = 0;
			attributeDescription[0].offset = offsetof(vert, pos);


			attributeDescription[1].binding = 0;
			attributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription[1].location = 1;
			attributeDescription[1].offset = offsetof(vert, uv);


			attributeDescription[2].binding = 0;
			attributeDescription[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription[2].location = 2;
			attributeDescription[2].offset = offsetof(vert, norm);


			attributeDescription[3].binding = 0;
			attributeDescription[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription[3].location = 3;
			attributeDescription[3].offset = offsetof(vert, tang);


			attributeDescription[4].binding = 0;
			attributeDescription[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription[4].location = 4;
			attributeDescription[4].offset = offsetof(vert, col);

			return attributeDescription;
		}
	};

	struct Model
	{
		bool isVBO = false;
		std::vector<vert> m_vertices;
		std::vector<uint32_t> m_indices;
		DeviceInfo* m_deviceInfo;

		Buffer m_vertexBuffer;
		Buffer m_IndexBuffer;

		glm::mat4 modelMatrix;

		glm::vec3 translate;
		glm::vec3 rotation;
		glm::vec3 scale;

		void CreateModelMatrix();

		bool MakeVBO(DeviceInfo* deviceInfo);
		bool BuildShape(phy::Shape* shape);
		void DrawIndexed(VkCommandBuffer cmdBuffer, VkPipelineLayout layout);
		void CreateSphere(float radius);
		void CreateBox(Vec3 dimensions);
		void ApplyColor(Vec3 col);

		void Cleanup();
	};

	struct RenderModel
	{
		Model* model;
		uint32_t uboByteOffset;	// The byte offset into the uniform buffer
		uint32_t uboByteSize;	// how much space we consume in the uniform buffer

		Vec3 pos;
		Quat orient;
	};
}