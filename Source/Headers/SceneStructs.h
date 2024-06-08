#pragma once
#include <stb_image.h>
namespace Renderer
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 texCoords;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, normal);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, color);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(Vertex, texCoords);

			return attributeDescriptions;
		}

		bool operator==(const Vertex& other) const {
			return position == other.position && color == other.color&& normal==other.normal && texCoords == other.texCoords;
		}

	};
	struct Triangle
	{
		std::array<Vertex, 3> verts;
	};

	struct Material
	{
		glm::vec3  diffuseColor;
		struct Specular
		{
			float exponent;
			float color;
		};
		float hasReflective;
		float hasRefractive;
		float emittance;
	};

	struct Texture
	{
		stbi_uc* pixels;
		int width;
		int height;
		int channels;
		Texture()
		{
			pixels = nullptr;
			width = height = channels = 0;
		}
	};

	struct PBRMaterial
	{
		glm::vec3 albedoColor;

		float roughness;
		float metalness;
		float clearCoat;

		Texture roughness_texture;
		Texture metalness_texture;
		Texture albedo_Texture;
		Texture normal_Texture;
	};

	struct MeshData
	{
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_index;

		Texture m_diffuseColorTexture;
		Texture m_normalTexture;
		Texture m_specularColorTexture;
	};

	struct Polygon
	{
		std::vector<Triangle> p_triangles;
		int material_id;
		Texture m_diffuseColorTexture;
		Texture m_normalTexture;
		Texture m_specularColorTexture;

		Polygon()
		{
			p_triangles = std::vector<Triangle>();
			m_diffuseColorTexture = Texture();
			m_normalTexture = Texture();
			m_specularColorTexture = Texture();
		};

	};
}