#pragma once
#include <stb_image.h>
#include "RendererInclude.h"
#include "Vulkan/Device.h"
#include "Vulkan/Texture.h"

namespace Renderer
{
	enum AlphaBlendMode
	{
		MODE_OPAQUE,
		MODE_MASK,
		MODE_BLEND,
	};
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 texCoord;

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
			attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}

		bool operator==(const Vertex& other) const {
			return position == other.position && color == other.color&& normal==other.normal && texCoord == other.texCoord;
		}

	};

	struct Material
	{
		Device* device = nullptr;
		AlphaBlendMode alphaMode = MODE_OPAQUE;
		float alphaCutoff = 1.0f;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		Texture* baseColorTexture = nullptr;
		Texture* metallicRoughnessTexture = nullptr;
		Texture* normalTexture = nullptr;
		Texture* occlusionTexture = nullptr;
		Texture* emissiveTexture = nullptr;

		Texture* specularGlossinessTexture;
		Texture* diffuseTexture;

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

		Material(Device* device) : device(device) {};
		void createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags);
	};



	struct MeshData
	{
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;

		Texture m_diffuseColorTexture;
		Texture m_normalTexture;
		Texture m_specularColorTexture;
	};

}