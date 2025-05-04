#include "Model.h"
#include "Math/Vector.h"
#include <algorithm>

#pragma warning( disable : 4996 )

/*
====================================================
FloatToByte
// Assumes a float between [-1,1]
====================================================
*/
unsigned char FloatToByte_n11(const float f) {
	int i = (int)(f * 127 + 128);
	return (unsigned char)i;
}


Vec3 Byte4toVec3(const unsigned char* val)
{
	Vec3 res;
	res.x = float(val[0]) / 255.0f;
	res.y = float(val[1]) / 255.0f;
	res.z = float(val[2]) / 255.0f;

	res.x = 2.0f * (res.x - 0.5f);
	res.y = 2.0f * (res.y - 0.5f);
	res.z = 2.0f * (res.z - 0.5f);

	return res;
}



namespace graphics
{

	bool Model::MakeVBO(DeviceInfo* deviceInfo)
	{
		m_deviceInfo = deviceInfo;
		/*TO Make VBO*/
		int buffer_size = (int)(sizeof(m_vertices[0]) * m_vertices.size());
		{
			Buffer stagingBuffer;

			stagingBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, m_vertices.data(), buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			m_vertexBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, nullptr, buffer_size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			
			//Copy Host to Device
			initializer::copyBuffer(stagingBuffer.buffer, m_vertexBuffer.buffer, buffer_size, m_deviceInfo->commandPool, m_deviceInfo->Device, m_deviceInfo->GraphicsQueue);

			stagingBuffer.Destroy();

		}
		buffer_size = (int)(sizeof(unsigned int) * m_indices.size());
		{
			Buffer stagingBuffer;

			stagingBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, m_indices.data(), buffer_size,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			m_IndexBuffer.Allocate(m_deviceInfo->PhysicalDevice, m_deviceInfo->Device, nullptr, buffer_size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			//Copy Host to Device
			initializer::copyBuffer(stagingBuffer.buffer, m_IndexBuffer.buffer, buffer_size, m_deviceInfo->commandPool, m_deviceInfo->Device, m_deviceInfo->GraphicsQueue);

			stagingBuffer.Destroy();
		}

		isVBO = true;

		return true;
	}

	bool Model::BuildShape(phy::Shape* shape)
	{
		if (shape == NULL)
			return false;

		if (shape->GetType() == phy::Shape::Sphere)
		{
			const phy::Shape_Sphere* shape_Sphere = (const phy::Shape_Sphere*)shape;
			m_vertices.clear();
			m_indices.clear();

			CreateSphere(shape_Sphere->m_radius);


		}

		if (shape->GetType() == phy::Shape::Box)
		{
			const phy::Shape_Box* shape_Box = (const phy::Shape_Box*)shape;
			m_vertices.clear();
			m_indices.clear();

			CreateBox(shape_Box->dimensions);
		}

		return true;
	}

	void Model::AssignMaterial(uint32_t ID)
	{
		m_materialID = ID;
	}

	void Model::ApplyColor(Vec3 col)
	{
		for (int i = 0; i < m_vertices.size(); i++)
		{
			m_vertices[i].col = glm::vec4(col.x, col.y, col.z, 1.0);
		}
	}

	void Model::CreateBox(Vec3 dimensions)
	{
		//m_vertices = 
		//{
		//
		//}

		m_vertices =
		{
			// Front Face
		 {{-0.5f, 0.5f,  0.5f}, {0, 0}, {0, 0, 1}, {1, 0, 0}}, // Bottom-left
		 {{ 0.5f, 0.5f,  0.5f}, {1, 0}, {0, 0, 1}, {1, 0, 0}}, // Bottom-right
		 {{-0.5f, - 0.5f,  0.5f}, {0, 1}, {0, 0, 1}, {1, 0, 0}}, // Top-left
		 {{ 0.5f, - 0.5f,  0.5f}, {1, 1}, {0, 0, 1}, {1, 0, 0}}, // Top-right

		 // Back Face
		 {{ 0.5f, 0.5f, -0.5f}, {0, 0}, {0, 0, -1}, {-1, 0, 0}}, // Bottom-left
		 {{-0.5f, 0.5f, -0.5f}, {1, 0}, {0, 0, -1}, {-1, 0, 0}}, // Bottom-right
		 {{ 0.5f, - 0.5f, -0.5f}, {0, 1}, {0, 0, -1}, {-1, 0, 0}}, // Top-left
		 {{-0.5f, - 0.5f, -0.5f}, {1, 1}, {0, 0, -1}, {-1, 0, 0}}, // Top-right

		 // Left Face
		 {{-0.5f, 0.5f, -0.5f}, {0, 0}, {-1, 0, 0}, {0, 0, 1}}, // Bottom-left
		 {{-0.5f, 0.5f,  0.5f}, {1, 0}, {-1, 0, 0}, {0, 0, 1}}, // Bottom-right
		 {{-0.5f, - 0.5f, -0.5f}, {0, 1}, {-1, 0, 0}, {0, 0, 1}}, // Top-left
		 {{-0.5f, - 0.5f,  0.5f}, {1, 1}, {-1, 0, 0}, {0, 0, 1}}, // Top-right

		 // Right Face
		 {{ 0.5f, 0.5f,  0.5f}, {0, 0}, {1, 0, 0}, {0, 0, -1}}, // Bottom-left
		 {{ 0.5f, 0.5f, -0.5f}, {1, 0}, {1, 0, 0}, {0, 0, -1}}, // Bottom-right
		 {{ 0.5f, -0.5f,  0.5f}, {0, 1}, {1, 0, 0}, {0, 0, -1}}, // Top-left
		 {{ 0.5f, -0.5f, -0.5f}, {1, 1}, {1, 0, 0}, {0, 0, -1}}, // Top-right

		 // Top Face
		 {{-0.5f,  -0.5f,  0.5f}, {0, 0}, {0, -1, 0}, {1, 0, 0}}, // Bottom-left
		 {{ 0.5f,  -0.5f,  0.5f}, {1, 0}, {0, -1, 0}, {1, 0, 0}}, // Bottom-right
		 {{-0.5f,  -0.5f, -0.5f}, {0, 1}, {0, -1, 0}, {1, 0, 0}}, // Top-left
		 {{ 0.5f,  -0.5f, -0.5f}, {1, 1}, {0, -1, 0}, {1, 0, 0}}, // Top-right

		 // Bottom Face
		 {{-0.5f, 0.5f, -0.5f}, {0, 0}, {0, 1, 0}, {1, 0, 0}}, // Bottom-left
		 {{ 0.5f, 0.5f, -0.5f}, {1, 0}, {0, 1, 0}, {1, 0, 0}}, // Bottom-right
		 {{-0.5f, 0.5f,  0.5f}, {0, 1}, {0, 1, 0}, {1, 0, 0}}, // Top-left
		 {{ 0.5f, 0.5f,  0.5f}, {1, 1}, {0, 1, 0}, {1, 0, 0}}, // Top-right


		};

		m_indices = {
				0, 1, 2, 2, 1, 3,       // Front
				4, 5, 6, 6, 5, 7,       // Back
				8, 9, 10, 10, 9, 11,    // Left
				12, 13, 14, 14, 13, 15, // Right
				16, 17, 18, 18, 17, 19, // Top
				20, 21, 22, 22, 21, 23  // Bottom
		};

		//Scaling on Given Dimensions
		for (int i = 0; i < m_vertices.size(); i++)
		{
			glm::vec3 pos = m_vertices[i].pos;
			m_vertices[i].pos = glm::vec3(pos.x * dimensions.x, pos.y * dimensions.y, pos.z * dimensions.z);
			m_vertices[i].col = glm::normalize(glm::vec4(glm::abs(pos), 1.0f));
		}
	}

	void Model::CreateSphere(float radius)
	{
		int latitudeSegments = 32;
		int longitudeSegments = 64;

		for (int lat = 0; lat <= latitudeSegments; lat++)
		{
			float Theta = lat * glm::pi<float>() / latitudeSegments; //Latitude form 0 to Pi
			float cosTheta = glm::cos(Theta);
			float sinTheta = glm::sin(Theta);

			for (int lon = 0; lon <= longitudeSegments; lon++)
			{
				float phi = lon * 2 * glm::pi<float>() / longitudeSegments;//Longitude from 0 to 2PI
				float sinphi = glm::sin(phi);
				float cosphi = glm::cos(phi);

				glm::vec3 normal = glm::vec3(cosphi * sinTheta, cosTheta, sinphi * sinTheta);
				glm::vec3 pos = normal * radius;
				glm::vec2 uv = glm::vec2(static_cast<float>(lon) / longitudeSegments, static_cast<float>(lat) / latitudeSegments);

				//Tangent
				glm::vec3 tangent = glm::normalize(glm::vec3(-sinphi, 0, cosphi));
				glm::vec4 col = glm::vec4(normal, 1.0f);

				m_vertices.push_back({ pos, uv, normal, tangent, col });
			}
		}

		for (int lat = 0; lat < latitudeSegments; lat++)
		{
			for (int lon = 0; lon < longitudeSegments; lon++)
			{
				int current = lat * (longitudeSegments + 1) + lon;
				int next = current + longitudeSegments + 1;

				//Triangle 1
				m_indices.push_back(current);
				m_indices.push_back(next);
				m_indices.push_back(current + 1);

				//Triangle 2
				m_indices.push_back(current + 1);
				m_indices.push_back(next);
				m_indices.push_back(next + 1);
			}
		}

	}

	void Model::CreateModelMatrix()
	{
		//Create Matrix on given input from the scene->body data;
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);

		modelMatrix = glm::mat4
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
				translate.x, -translate.y, translate.z, 1.0f
			}
		};
	}

	void Model::DrawIndexed(VkCommandBuffer cmdBuffer, VkPipelineLayout layout)
	{
		PushConstant pushData = { m_materialID , modelMatrix };

		VkBuffer vertexBuffer[] = { m_vertexBuffer.buffer };
		VkDeviceSize offset[] = { 0 };
		vkCmdPushConstants(cmdBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pushData);

		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffer, offset);
		vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		
		vkCmdDrawIndexed(cmdBuffer, (uint32_t)m_indices.size(), 1, 0, 0, 0);
	}

	void Model::Cleanup()
	{
		if (isVBO)
		{
			m_IndexBuffer.Destroy();
			m_vertexBuffer.Destroy();
		}
	}
}