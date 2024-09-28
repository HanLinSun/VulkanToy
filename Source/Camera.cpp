#include "Camera.h"

namespace Renderer
{
	Camera::Camera(Device* device, float aspectRatio) :m_device(device)
	{
		r = 20.0f;
		theta = 45.0f;
		phi = -45.0f;


		m_position = glm::vec3(0.0f, 20.f, 120.f);
		m_upVector = glm::vec3(0.0f, 1.0f, 0.0f);
		
		m_lookAtPoint = glm::vec3(1.0f, 0.0f, 0.0);
		m_forwardVector = glm::normalize(m_lookAtPoint -m_position);
		m_rightVector = glm::cross(m_forwardVector,m_upVector);

		m_aspectRatio = aspectRatio;
		m_nearClipPlane = 0.1f;
		m_farClipPlane = 1500.0f;
		m_fov = 60.0f;

		UpdateViewMatrix();
		UpdateProjectionMatrix();

		m_cameraBufferObject.viewMatrix = m_viewMatrix;
		m_cameraBufferObject.projectionMatrix = m_projectionMatrix;

		//m_cameraBufferObject.viewMatrix = glm::lookAt(glm::vec3(0.0f, 20.0f, 120.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//m_cameraBufferObject.projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearClipPlane, m_farClipPlane);
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

		glm::mat4 viewMat_orient = { glm::vec4(m_rightVector,0),glm::vec4(m_upVector,0),glm::vec4(m_forwardVector,0),glm::vec4(0,0,0,1) };
		m_viewMatrix = rotationMatrix * translateMatrix;
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
		m_position.x += m_movingSpeed * deltaTime;
		UpdateViewMatrix();
	}
	void Camera::UpdateTransform_Y(float  deltaTime)
	{
		m_position.y += m_movingSpeed * deltaTime;
		UpdateViewMatrix();
	}
	void Camera::UpdateTransform_Z(float  deltaTime)
	{
		m_position.z += m_movingSpeed * deltaTime;
		UpdateViewMatrix();
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