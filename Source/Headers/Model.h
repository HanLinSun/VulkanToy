#pragma once
#include "Vulkan/Device.h"
#include "SceneStructs.h"
#include "Vulkan/Texture.h"
namespace Renderer
{

	struct ModelUniformBuffer
	{

	};

	class Model
	{
	public:
		Model() = delete;
		Model(Device* device, VkCommandPool commandPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		virtual ~Model();

		void SetTexture(VkImage texture);

		const std::vector<Vertex>& GetVertices() const;

		VkBuffer GetVertexBuffer() const;

		const std::vector<uint32_t>& GetIndices() const;

		VkBuffer GetIndexBuffer() const;

		const ModelUniformBuffer& GetModelUniformBuffer() const;

		VkBuffer GetModelBuffer() const;
		VkImageView GetTextureView() const;
		VkSampler GetTextureSampler() const;

	protected:
		Device* m_device;
		std::vector<Vertex> m_vertices;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;

		std::vector<uint32_t> m_indices;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;

		std::vector<Texture> m_textures;
		std::vector<Material> m_materials;
	};
}