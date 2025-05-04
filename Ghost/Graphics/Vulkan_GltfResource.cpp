#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif

#include "Vulkan_GltfResource.h"

namespace graphics
{
	bool GltfModel::loadGLTF(const std::string& filename, DeviceInfo* deviceInfo)
	{
		tinygltf::Model	gltfInput;
		tinygltf::TinyGLTF gltfContext;
		std::string error, warning;

		m_deviceInfo = deviceInfo;

		bool fileLoad  =	  gltfContext.LoadASCIIFromFile(&gltfInput, &error, &warning, filename);
		std::vector<uint32_t> indexBuffer;
		std::vector<vert>	  vertexBuffer;

		if (fileLoad)
		{
			const tinygltf::Scene& scene = gltfInput.scenes[0];
			for (size_t i = 0; i < scene.nodes.size(); i++)
			{
				const tinygltf::Node& node = gltfInput.nodes[scene.nodes[i]];
				loadNode(node, gltfInput, nullptr, indexBuffer, vertexBuffer);
			}
		}

		else
		{
			throw std::runtime_error("Could not open the glTF file.\n\nMake sure the assets submodule has been checked out and is up - to - date.");
		}

		int Buffer_size = (int)(sizeof(vertexBuffer[0])) * static_cast<int>(vertexBuffer.size());
		// Making Vertex Buffer
		{
			Buffer stagingBuffer;

			stagingBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, vertexBuffer.data(), Buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			m_vertexBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, nullptr, Buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			//Copy Host to Device
			initializer::copyBuffer(stagingBuffer.buffer, m_vertexBuffer.buffer, Buffer_size, m_deviceInfo->commandPool, m_deviceInfo->Device, m_deviceInfo->GraphicsQueue);
			stagingBuffer.Destroy();
		}

		Buffer_size = (int)(sizeof(uint32_t)) * indexBuffer.size();
		//Making Index Buffer
		{
			Buffer stagingBuffer;

			stagingBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, indexBuffer.data(), Buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			m_indexBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, nullptr, Buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			initializer::copyBuffer(stagingBuffer.buffer, m_indexBuffer.buffer, Buffer_size, m_deviceInfo->commandPool, m_deviceInfo->Device, m_deviceInfo->GraphicsQueue);

			stagingBuffer.Destroy();
		}

		return true;
	}

	void GltfModel::loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& IndexBuffer, std::vector<vert>& VertexBuffer)
	{
		Node* node = new Node();

		node->modelMatrix = glm::mat4(1.0f);
		node->parent = parent;

		//Get the local Node Matrix
		if (inputNode.translation.size() == 3)
		{
			node->modelMatrix = glm::translate(node->modelMatrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
		}

		if (inputNode.rotation.size() == 4)
		{
			glm::quat q = glm::make_quat(inputNode.rotation.data());
			node->modelMatrix *= glm::mat4(q);
		}

		if (inputNode.scale.size() == 3)
		{
			node->modelMatrix = glm::scale(node->modelMatrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
		}

		if (inputNode.matrix.size() == 16)
		{
			node->modelMatrix = glm::make_mat4x4(inputNode.matrix.data());

		}

		//Load Nodes Children
		if(inputNode.children.size() > 0)
		{
			for (size_t i = 0; i < inputNode.children.size(); i++)
			{
				loadNode(input.nodes[inputNode.children[i]], input, node, IndexBuffer, VertexBuffer);
			}
		}

		//Now we are loading indices and vertices from buffer
		//this is done via accessors and buffer views in gltf
		if (inputNode.mesh > -1)
		{
			const tinygltf::Mesh& mesh = input.meshes[inputNode.mesh];
			// Iterate through all primitives of this node's mesh
			for (int i = 0; i < mesh.primitives.size(); i++)
			{
				const tinygltf::Primitive& gltfprimitive = mesh.primitives[i];
				uint32_t firstIndex = static_cast<uint32_t>(IndexBuffer.size());
				uint32_t vertexStart = static_cast<uint32_t>(VertexBuffer.size());
				uint32_t indexCount = 0;

				//Vertices
				{
					const float* positionBuffer = nullptr;
					const float* normalBuffer = nullptr;
					const float* texCoordBuffer = nullptr;
					const float* tangentBuffer = nullptr;
					size_t VertexCount = 0;

					//Get Buffer data for Vertex Position
					if (gltfprimitive.attributes.find("POSITION") != gltfprimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor	= input.accessors[gltfprimitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& view	= input.bufferViews[accessor.bufferView];
						positionBuffer						= reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						VertexCount							= accessor.count;
					}
					// Get the Buffer data for Normal
					if (gltfprimitive.attributes.find("NORMAL") != gltfprimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = input.accessors[gltfprimitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& view   = input.bufferViews[accessor.bufferView];
						normalBuffer					   = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));

					}
					// Get the buffer data for Texture 
					// glTF supports multiple sets, we only load the first one
					if (gltfprimitive.attributes.find("TEXCOORD_0") != gltfprimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = input.accessors[gltfprimitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& view   = input.bufferViews[accessor.bufferView];
						texCoordBuffer					   = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}
					// Get the buffer data for Tangent
					if (gltfprimitive.attributes.find("TANGENT") != gltfprimitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = input.accessors[gltfprimitive.attributes.find("TANGENT")->second];
						const tinygltf::BufferView& view   = input.bufferViews[accessor.bufferView];
						tangentBuffer					   = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					for (size_t v = 0; v < VertexCount; v++)
					{
						vert _vert{};
						_vert.pos  = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
						_vert.norm = glm::normalize(glm::vec3(normalBuffer ? glm::make_vec3(&normalBuffer[v * 3]) : glm::vec3(0.0f)));
						_vert.uv   = texCoordBuffer ? glm::make_vec2(&texCoordBuffer[v * 2]) : glm::vec2(0.0f);
						_vert.col  = glm::vec4(0.5f);
						_vert.tang = tangentBuffer ? glm::make_vec4(&tangentBuffer[v * 4]) : glm::vec4(0.0f);

						VertexBuffer.push_back(_vert);
					}
				}

				//Indices
				{
					const tinygltf::Accessor& accessor = input.accessors[gltfprimitive.indices];
					const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

					indexCount += static_cast<uint32_t>(accessor.count);

					// glTF supports different component types of indices
					switch (accessor.componentType)
					{
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++)
						{
							IndexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}

					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++)
						{
							IndexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}

					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++)
						{
							IndexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}

					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}

				Primitive primitive{};
				primitive.firstIndex = firstIndex;
				primitive.indexCount = indexCount;
				primitive.materialIndex = gltfprimitive.material;
				node->mesh.primitive.push_back(primitive);

			}
		}

		if (parent) {
			parent->children.push_back(node);
		}
		else {
			nodes.push_back(node);
		}


	}

	void GltfModel::drawNode(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, Node* node)
	{
		if (node->mesh.primitive.size() > 0)
		{
			// Pass the node's matrix via push constants
			// Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
			glm::mat4 nodeMatrix = node->modelMatrix;
			Node* currentParent = node->parent;
			while (currentParent)
			{
				nodeMatrix =  currentParent->modelMatrix * nodeMatrix;
				currentParent = currentParent->parent;
			}

			nodeMatrix = mainModelMatrix * nodeMatrix;

			PushConstant push = { m_materialID , nodeMatrix };

			vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push);
			for (Primitive& primitive : node->mesh.primitive)
			{
				vkCmdDrawIndexed(cmdBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
			}
		}

		for (Node* _node : node->children)
		{
			drawNode(cmdBuffer, pipelineLayout, _node);
		}
	}

	void GltfModel::draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout)
	{
		// All vertices and indices are stored in single buffers, so we only need to bind once
		VkDeviceSize offset[1] = { 0 };
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_vertexBuffer.buffer, offset);
		vkCmdBindIndexBuffer(cmdBuffer, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		//Render all nodes
		for (Node* node : nodes)
		{
			drawNode(cmdBuffer, pipelineLayout, node);
		}
	}

	void GltfModel::CreateModelMatrix()
	{
		//Create Matrix on given input from the scene->body data;
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);

		mainModelMatrix = glm::mat4
		{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f
			},
			{
				translate.x, translate.y, translate.z, 1.0f
			}
		};

		mainModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f)) * mainModelMatrix;
	}

	void GltfModel::AssignMaterial(uint32_t ID)
	{
		m_materialID = ID;
	}

	void GltfModel::Destroy()
	{
		for (Node* node : nodes)
		{
			delete node;
		}

		m_vertexBuffer.Destroy();
		m_indexBuffer.Destroy();
	}
}