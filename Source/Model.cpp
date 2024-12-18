#include "Model.h"
#include "Vulkan/BufferUtils.h"
namespace Renderer
{
	ModelGroup::ModelGroup() {};

	void ModelGroup::AddModel(Model* _model)
	{
		m_models.push_back(std::unique_ptr<Model>(_model));
	}

    Model* ModelGroup::GetModelAt(int idx) const
	{
		return m_models[idx].get();
	}

	void ModelGroup::AddModels(std::vector<Model*>& _models)
	{
		for (auto& model : _models)
		{
			m_models.push_back(std::unique_ptr<Model>(model));
		}
	}

	size_t ModelGroup::GetModelSize() const
	{
		return m_models.size();
	}

	void ModelGroup::DestroyVKResources()
	{
		for (int i = 0; i < this->GetModelSize(); i++)
		{
			auto model = this->GetModelAt(i);
			model->DestroyVKResources();
		}

		for (int i = 0; i < this->GetMaterialSize(); i++)
		{
			auto material = this->GetMaterial(i);
			material->DestroyResources();
		}
	}


	Model::Model(Device* device, VkCommandPool commandPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,std::shared_ptr<Material> mat, const std::vector<Triangle>& m_triangles)
		:m_device(device), m_vertices(vertices), m_indices(indices), m_material(mat),m_triangles(m_triangles)
	{
		if (m_vertices.size() > 0)
		{
			BufferUtils::CreateBufferFromData(device, commandPool, this->m_vertices.data(), m_vertices.size() * sizeof(Vertex),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_vertexBuffer, m_vertexBufferMemory);
		}

		if (m_indices.size() > 0)
		{
			BufferUtils::CreateBufferFromData(device, commandPool, this->m_indices.data(), m_indices.size() * sizeof(uint32_t),
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_indexBuffer, m_indexBufferMemory);
		}
		m_modelBufferObject.modelMatrix = glm::mat4(1.0f);
		BufferUtils::CreateBufferFromData(device, commandPool, &m_modelBufferObject, sizeof(ModelBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_modelUniformBuffer, m_modelUniformBufferMemory);
	}

	void Model::DestroyVKResources()
	{
		if (m_indices.size() > 0) {
			vkDestroyBuffer(m_device->GetVkDevice(), m_indexBuffer, nullptr);
			vkFreeMemory(m_device->GetVkDevice(), m_indexBufferMemory, nullptr);
		}

		if (m_vertices.size() > 0) {
			vkDestroyBuffer(m_device->GetVkDevice(), m_vertexBuffer, nullptr);
			vkFreeMemory(m_device->GetVkDevice(), m_vertexBufferMemory, nullptr);
		}

		vkDestroyBuffer(m_device->GetVkDevice(), m_modelUniformBuffer, nullptr);
		vkFreeMemory(m_device->GetVkDevice(), m_modelUniformBufferMemory, nullptr);
	}

	Model::~Model()
	{
		
	}

	const std::vector<Vertex>& Model::GetVertices() const {
		return m_vertices;
	}

	VkBuffer Model::GetVertexBuffer() const {
		return m_vertexBuffer;
	}

	const std::vector<uint32_t>& Model::GetIndices() const {
		return m_indices;
	}

	VkBuffer Model::GetIndexBuffer() const {
		return m_indexBuffer;
	}

	const ModelBufferObject& Model::GetModelBufferObject() const {
		return m_modelBufferObject;
	}

	VkBuffer Model::GetModelUniformBuffer() const {
		return m_modelUniformBuffer;
	}

	Material* Model::GetMaterial() const
	{
		return m_material.get();
	}

	void Model::SetMaterial(Material* mat)
	{
		m_material = std::make_unique<Material>(std::move(mat));
	}

	size_t ModelGroup::GetMaterialSize() const
	{
		return m_materials.size();
	}

	Material* ModelGroup::GetMaterial(int idx) const
	{
		return m_materials[idx].get();
	}
	void ModelGroup::AddMaterial(std::shared_ptr<Material> mat)
	{
		m_materials.push_back(mat);
	}

	void ModelGroup::AddMaterial(Material* mat)
	{
		m_materials.push_back(std::shared_ptr<Material>(mat));
	}



 }
