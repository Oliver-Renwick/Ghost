#pragma once

#include "Vulkan_GltfResource.h"
#include "Math/Mat.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Physics/Shapes/Shapes.h"
#include "Buffer.h"
#include "Physics/Shapes/Sphere_Shape.h"
#include "Physics/Shapes/Box_Shape.h"
#include "Vulkan_Init.h"
#include <vulkan/vulkan.h>
#include <array>




namespace graphics
{

	

	struct Model
	{
		bool isVBO = false;
		std::vector<vert> m_vertices;
		std::vector<uint32_t> m_indices;
		DeviceInfo* m_deviceInfo;

		Buffer m_vertexBuffer;
		Buffer m_IndexBuffer;

		glm::mat4 modelMatrix;
		uint32_t  m_materialID = 0;

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
		void AssignMaterial(uint32_t ID);

		void Cleanup();
	};

	struct RenderModel
	{
		Model* model = nullptr;
		GltfModel* gltfModel = nullptr;

		uint32_t uboByteOffset;	// The byte offset into the uniform buffer
		uint32_t uboByteSize;	// how much space we consume in the uniform buffer

		Vec3 pos;
		Quat orient;
	};
}