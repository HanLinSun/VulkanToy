#pragma once
#include "Vulkan/Device.h"
#include "SceneStructs.h"
#include "Vulkan/Texture.h"
#include "Material.h"
#include <glm/glm.hpp>
namespace Renderer
{

	struct ModelBufferObject
	{
		glm::mat4 modelMatrix;
	};
	enum TextureType
	{
		BaseColor =0,
		Metallic=1,
		Roughness=2,
		Albedo=3,
		Glossiness=4,
		Specular=5,
		AmbientOcclusion=6,
		Emissive=7, 
		Opacity=8,
		Displacement=9,
		Anisotropic=10,
	};

	class Model
	{
	public:
		Model() = delete;
		Model(Device* device, VkCommandPool commandPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		Model(Model& model);

		virtual ~Model();

		void SetTexture(Texture texture,TextureType type);

		const std::vector<Vertex>& GetVertices() const;

		VkBuffer GetVertexBuffer() const;

		const std::vector<uint32_t>& GetIndices() const;

		VkBuffer GetIndexBuffer() const;

		const ModelBufferObject& GetModelBufferObject() const;

		VkBuffer GetModelUniformBuffer() const;

		Texture GetBindTextureOfType(TextureType type) const;

	protected:
		Device* m_device;
		std::vector<Vertex> m_vertices; 
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;

		std::vector<uint32_t> m_indices;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;

		VkBuffer m_modelUniformBuffer;
		VkDeviceMemory m_modelUniformBufferMemory;

		ModelBufferObject m_modelBufferObject;

		std::vector<Texture> m_textures;

		std::vector<Material> m_materials;
	};
}