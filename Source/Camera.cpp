#include "Camera.h"
#include <glm/gtx/transform.hpp>

namespace Renderer
{
	Camera::Camera(Device* device, float aspectRatio) :m_device(device)
	{
		r = 20.0f;
		theta = 45.0f;
		phi = -45.0f;

		m_position = glm::vec4(0.0f, 20.f, 100.f,1.0f);
		m_upVector = glm::vec4(0.0f, 1.0f, 0.0f,0.0f);
		
		m_forwardVector = glm::vec4(0.0f, 0.0f, -1.0f,0.0f);
		m_rightVector = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

		m_aspectRatio = aspectRatio;
		m_nearClipPlane = 0.1f;
		m_farClipPlane = 2500.0f;
		m_fov = 60.0f;

		UpdateViewMatrix();
		UpdateProjectionMatrix();

		m_cameraBufferObject.viewMatrix = m_viewMatrix;
		m_cameraBufferObject.projectionMatrix = m_projectionMatrix;

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

	glm::mat4 Camera::GetProjectionMatrix()
	{
		return m_projectionMatrix;
	}

	glm::mat4 Camera::GetViewmatrix()
	{
		return m_viewMatrix;
	}

	void Camera::UpdateViewMatrix()
	{
		glm::mat4 rotationMatrix;
		rotationMatrix[0] = glm::vec4(m_rightVector.x, m_upVector.x, m_forwardVector.x, 0);
		rotationMatrix[1] = glm::vec4(m_rightVector.y, m_upVector.y, m_forwardVector.y, 0);
		rotationMatrix[2] = glm::vec4(m_rightVector.z, m_upVector.z, m_forwardVector.z, 0);
		rotationMatrix[3] = glm::vec4(0, 0, 0, 1);

		glm::mat4 translateMatrix;
		translateMatrix[0] = glm::vec4(1, 0, 0, 0);
		translateMatrix[1] = glm::vec4(0, 1, 0, 0);
		translateMatrix[2] = glm::vec4(0, 0, 1, 0);
		translateMatrix[3] = glm::vec4(-m_position.x, -m_position.y, -m_position.z, 1);

		m_viewMatrix = rotationMatrix * translateMatrix;
	}

	void Camera::UpdateBufferMemory()
	{
		m_cameraBufferObject.viewMatrix = m_viewMatrix;
		m_cameraBufferObject.projectionMatrix = m_projectionMatrix;
		m_cameraBufferObject.projectionMatrix[1][1] *= -1;

		memcpy(m_mappedData, &m_cameraBufferObject, sizeof(CameraUniformBuffer));
	}

	void Camera::UpdateProjectionMatrix()
	{
		//Column compute
		glm::mat4 _projectionMatrix;
		float radian = glm::radians(m_fov/2);
		float s = 1 / tan(radian);
		float p = m_farClipPlane / (m_farClipPlane - m_nearClipPlane);
		float q = m_farClipPlane * m_nearClipPlane / (m_nearClipPlane- m_farClipPlane);
		_projectionMatrix[0] = glm::vec4(s / m_aspectRatio, 0, 0, 0);
		_projectionMatrix[1] = glm::vec4(0, s, 0, 0);
		_projectionMatrix[2] = glm::vec4(0, 0, p, 1);
		_projectionMatrix[3] = glm::vec4(0, 0, q, 0);

		m_projectionMatrix = _projectionMatrix;
	}

	void Camera::UpdateTransform_X(float deltaTime)
	{
		m_position+= m_movingSpeed * deltaTime*m_rightVector;
		UpdateViewMatrix();
	}
	void Camera::UpdateTransform_Y(float  deltaTime)
	{
		m_position+= m_movingSpeed * deltaTime*m_upVector;
		UpdateViewMatrix();
	}
	void Camera::UpdateTransform_Z(float  deltaTime)
	{
		m_position+= m_movingSpeed * deltaTime*m_forwardVector;
		UpdateViewMatrix();
	}

	void Camera::RotateAroundForwardAxis(float degree)
	{
		glm::vec3 axis = glm::vec3(m_forwardVector.x, m_forwardVector.y, m_forwardVector.z);
		glm::mat4 _rotationMatrix = glm::rotate(degree, axis);
		m_upVector = _rotationMatrix * m_upVector;
		m_rightVector = _rotationMatrix * m_rightVector;
	}

	void Camera::RotateAroundRightAxis(float degree)
	{
		glm::vec3 axis = glm::vec3(m_rightVector.x, m_rightVector.y, m_rightVector.z);
		glm::mat4 _rotationMatrix = glm::rotate(degree, axis);
		m_forwardVector = _rotationMatrix * m_forwardVector;
		m_upVector = _rotationMatrix * m_upVector;
	}

	void Camera::RotateAroundUpAxis(float degree)
	{
		glm::vec3 axis = glm::vec3(m_upVector.x, m_upVector.y, m_upVector.z);
		glm::mat4 _rotationMatrix = glm::rotate(degree, axis);
		m_forwardVector = _rotationMatrix * m_forwardVector;
		m_rightVector = _rotationMatrix * m_rightVector;
	}

	Camera::~Camera() {
		vkUnmapMemory(m_device->GetVkDevice(), m_bufferMemory);
		vkDestroyBuffer(m_device->GetVkDevice(), m_buffer, nullptr);
		vkFreeMemory(m_device->GetVkDevice(), m_bufferMemory, nullptr);
	}

	
}