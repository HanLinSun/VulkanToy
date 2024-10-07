#pragma once
#include <glm/glm.hpp>
#include <Vulkan/Device.h>
#include <Material.h>
#include "SceneStructs.h"
namespace Renderer
{
	class Cube
	{
	public:
		Cube(Device* device, VkCommandPool commandPool);
		~Cube();

		void SetTransformMatrix(glm::mat4 transMat);
		void SetScaleMatrix(glm::mat4 scaleMat);
		void SetRotationMatrix(glm::mat4 rotMat);
		void SetMaterial(Material _mat);

	private:
		Device* m_device;
		VkCommandPool m_commandPool;

		std::vector<Vertex> m_vertices;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;

		std::vector<uint32_t> m_indices;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;

		Material m_material;

		glm::mat4 m_transformMatrix;
		glm::mat4 m_rotationMatrix;
		glm::mat4 m_scaleMatrix;
	};
 }
