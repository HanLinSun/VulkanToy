// Copyright 2024 Hanlin Sun
#include "Model.h"
#include "Vulkan/BufferUtils.h"
#include <glm/gtc/matrix_inverse.hpp>

#define PI 3.1415926535897932384626422832795028841971f
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
	}

	void ModelGroup::buildTransformationMatrix(glm::vec3& trans, glm::vec3& rot, glm::vec3& scale)
	{
		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), trans);
		glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), rot.x * (float)PI / 180, glm::vec3(1, 0, 0));
		rotationMat = rotationMat * glm::rotate(glm::mat4(1.0f), rot.y * (float)PI / 180, glm::vec3(0, 1, 0));
		rotationMat = rotationMat * glm::rotate(glm::mat4(1.0f), rot.z * (float)PI / 180, glm::vec3(0, 0, 1));
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

		m_transformMatrix = translationMat * rotationMat * scaleMat;
	}

	glm::mat4 ModelGroup::GetTransformMatrix()
	{
		return m_transformMatrix;
	}

	void ModelGroup::SetTransformMatrix(glm::mat4& _transformMatrix)
	{
		m_transformMatrix = _transformMatrix;
		SetModelTransformMatrix();
	}

	Model::Model(Device* device, VkCommandPool commandPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,std::shared_ptr<Material> mat, const std::vector<Triangle>& m_triangles)
		:m_device(device), m_vertices(vertices), m_indices(indices), m_material(mat),m_triangles(m_triangles){
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

		m_transformMatrix = glm::mat4(1.0f);
		m_modelBufferObject.transformMatrix = m_transformMatrix;

		BufferUtils::CreateBuffer(device, sizeof(ModelBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_modelUniformBuffer, m_modelUniformBufferMemory);

		vkMapMemory(device->GetVkDevice(), m_modelUniformBufferMemory, 0, sizeof(ModelBufferObject), 0, &m_mappedData);
		memcpy(m_mappedData, &m_modelBufferObject, sizeof(ModelBufferObject));
	}

	void ModelGroup::SetModelTransformMatrix()
	{
		for (int i = 0; i < m_models.size(); i++)
		{
			m_models[i]->SetTransformMatrix(m_transformMatrix);
		}
	}

	std::vector<Triangle> ModelGroup::GetAllModelTriangles()
	{
		std::vector<Triangle> triangleVector(0);
		for (auto& model : m_models)
		{
			const auto& triangles = model->GetTriangles(); // Get the vector of triangles
			triangleVector.insert(triangleVector.end(), triangles.begin(), triangles.end());
		}
		return triangleVector;
	}

	int ModelGroup::GetAllTriangleSize()
	{
		int triangleSize = 0;
		for (auto& model : m_models)
		{
			triangleSize += model->GetTriangles().size();
		}
		return triangleSize;
	}

	glm::mat4 Model::GetTransformMatrix()
	{
		return m_transformMatrix;
	}

	int Model::GetTriangleSize()
	{
		return m_triangles.size();
	}

	glm::mat4 Model::GetInverseTransposeMatrix()
	{
		return m_inverseTransposeMatrix;
	}

	void Model::SetTransformMatrix(glm::mat4 transformMat)
	{
		m_transformMatrix = transformMat;
		m_modelBufferObject.transformMatrix=transformMat;

		//Update uniform buffer
		memcpy(m_mappedData, &m_modelBufferObject, sizeof(ModelBufferObject));
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

	std::vector<Triangle> Model::GetTriangles()
	{
		return m_triangles;
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

 }
