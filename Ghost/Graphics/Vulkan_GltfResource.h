#pragma once



#include "tiny_gltf.h"

#include "Vulkan_Init.h"
#include "Buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

struct PushConstant
{
	uint32_t material_Index = 0;
	alignas(16) glm::mat4 modelMatrix;
};

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

namespace graphics
{
	struct GltfModel
	{
		struct Primitive
		{
			uint32_t firstIndex;
			uint32_t indexCount;
			int32_t  materialIndex;
		};

		struct Mesh
		{
			std::vector<Primitive> primitive;
		};

		struct Node
		{
			Node* parent = nullptr;
			std::vector<Node*> children;
			Mesh mesh;
			glm::mat4 modelMatrix;

			~Node()
			{
				for (Node* child : children)
				{
					delete child;
				}
			}
		};

		glm::vec3 translate;
		glm::vec3 rotation;
		glm::vec3 scale;
		glm::mat4 mainModelMatrix;
		uint32_t m_materialID = 0;
		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
		std::vector<Node*> nodes;
		DeviceInfo* m_deviceInfo = nullptr;
		bool loadGLTF(const std::string& filename, DeviceInfo* deviceInfo);
		void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& inputModel, Node* parent, std::vector<uint32_t>& IndexBuffer, std::vector<vert>& VertexBuffer);
		void draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout);
		void drawNode(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, Node* node);
		void CreateModelMatrix();
		void AssignMaterial(uint32_t ID);
		void Destroy();
	};
}