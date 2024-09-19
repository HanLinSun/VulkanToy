#pragma once
#include "Vulkan/Device.h"
#include "SceneStructs.h"
#include "Vulkan/Texture.h"
#include "Material.h"
#include <glm/glm.hpp>
#include <map>
namespace Renderer
{

	struct ModelBufferObject
	{
		glm::mat4 modelMatrix;
	};


	class Model
	{
	public:
		Model() = delete;
		Model(Device* device, VkCommandPool commandPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Material* mat);
		Model(Model& model);

		~Model();


		const std::vector<Vertex>& GetVertices() const;

		VkBuffer GetVertexBuffer() const;

		const std::vector<uint32_t>& GetIndices() const;

		VkBuffer GetIndexBuffer() const;

		const ModelBufferObject& GetModelBufferObject() const;

		VkBuffer GetModelUniformBuffer() const;
		Material* GetMaterial() const;
		void SetMaterial(Material* mat);

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
		std::unique_ptr<Material> m_material;
		

	};
}