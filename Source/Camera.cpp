#include "Camera.h"

namespace Renderer
{
	Camera::Camera(Device* device, float aspectRatio) :m_device(device)
	{
		r = 20.0f;
		theta = 45.0f;
		phi = -45.0f;

		m_cameraBufferObject.viewMatrix = glm::lookAt(glm::vec3(0.0f, 1.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_cameraBufferObject.projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
		m_cameraBufferObject.projectionMatrix[1][1] *= -1; // y-coordinate is flipped

		BufferUtils::CreateBuffer(device, sizeof(CameraUniformBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_bufferMemory);

		vkMapMemory(device->GetVkDevice(), m_bufferMemory, 0, sizeof(CameraUniformBuffer), 0, &m_mappedData);
		memcpy(m_mappedData, &m_cameraBufferObject, sizeof(CameraUniformBuffer));
	}

	VkBuffer Camera::GetBuffer() const
	{
		return m_buffer;
	}

	void Camera::UpdateOrbit(float deltaX, float deltaY, float deltaZ)
	{
		theta += deltaX;
		phi += deltaY;
		r = glm::clamp(r - deltaZ, 1.0f, 50.0f);

		float radTheta = glm::radians(theta);
		float radPhi = glm::radians(phi);

		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), radTheta, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), radPhi, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 finalTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)) * rotation * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, r));

		m_cameraBufferObject.viewMatrix = glm::inverse(finalTransform);

		memcpy(m_mappedData, &m_cameraBufferObject, sizeof(CameraUniformBuffer));
	}

	Camera::~Camera() {
		vkUnmapMemory(m_device->GetVkDevice(), m_bufferMemory);
		vkDestroyBuffer(m_device->GetVkDevice(), m_buffer, nullptr);
		vkFreeMemory(m_device->GetVkDevice(), m_bufferMemory, nullptr);
	}

	
}