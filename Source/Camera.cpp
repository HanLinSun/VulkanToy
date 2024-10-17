#include "Camera.h"
#include <glm/gtx/transform.hpp>

namespace Renderer
{
	Camera::Camera(Device* device, float aspectRatio) :m_device(device)
	{
		r = 20.0f;
		theta = 45.0f;
		phi = -45.0f;

		m_position_W = glm::vec4(0.0f, 20.f, 100.f,1.0f);
		m_upVector_W = glm::vec4(0.0f, 1.0f, 0.0f,0.0f);
		
		m_forwardVector_W = glm::vec4(0.0f, 0.0f, -1.0f,0.0f);
		m_rightVector_W = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

		m_aspectRatio = aspectRatio;
		m_nearClipPlane = 0.1f;
		m_farClipPlane = 5500.0f;
		m_fov = 60.0f;

		UpdateViewMatrix(Handedness::RightHanded);
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

	inline glm::vec3 Camera::Get3DVectorComponent(const glm::vec4 vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}
	inline glm::vec4 Camera::Set3DVectorComponent(const glm::vec3 vec)
	{
		return glm::vec4(vec.x, vec.y, vec.z, 1);
	}

	glm::mat4 Camera::GetProjectionMatrix()
	{
		return m_projectionMatrix;
	}

	glm::mat4 Camera::GetViewmatrix()
	{
		return m_viewMatrix;
	}

	void Camera::UpdateViewMatrix(Handedness hand)
	{
		//glm::mat4 rotationMatrix;
		//rotationMatrix[0] = glm::vec4(m_rightVector.x, m_upVector.x, m_forwardVector.x, 0);
		//rotationMatrix[1] = glm::vec4(m_rightVector.y, m_upVector.y, m_forwardVector.y, 0);
		//rotationMatrix[2] = glm::vec4(m_rightVector.z, m_upVector.z, m_forwardVector.z, 0);
		//rotationMatrix[3] = glm::vec4(0, 0, 0, 1);

		//glm::mat4 translateMatrix;
		//translateMatrix[0] = glm::vec4(1, 0, 0, 0);
		//translateMatrix[1] = glm::vec4(0, 1, 0, 0);
		//translateMatrix[2] = glm::vec4(0, 0, 1, 0);
		//translateMatrix[3] = glm::vec4(-m_position.x, -m_position.y, -m_position.z, 1);

		m_viewMatrix[0] =glm::vec4(m_rightVector_W.x, m_upVector_W.x, m_forwardVector_W.x, 0);
		m_viewMatrix[1] = glm::vec4(m_rightVector_W.y, m_upVector_W.y, m_forwardVector_W.y, 0);
		m_viewMatrix[2] = glm::vec4(m_rightVector_W.z, m_upVector_W.z, m_forwardVector_W.z, 0);
		m_viewMatrix[3] = glm::vec4(-glm::dot(m_rightVector_W,m_position_W), -glm::dot(m_upVector_W, m_position_W), -glm::dot(m_forwardVector_W, m_position_W),1);
	}

	void Camera::UpdateViewMatrixFromLookAt(Handedness hand)
	{
		m_forwardVector_W = (hand == Handedness::RightHanded) ? glm::normalize(m_position_W - m_lookTarget_W) : glm::normalize(m_lookTarget_W - m_position_W);

		glm::vec3 rightVec = Get3DVectorComponent(m_rightVector_W);
		glm::vec3 upVec = Get3DVectorComponent(m_upVector_W);
		glm::vec3 forwardVec = Get3DVectorComponent(m_forwardVector_W);

		rightVec = glm::normalize(glm::cross(upVec, forwardVec));
		upVec = glm::normalize(glm::cross(forwardVec, rightVec));

		//vector<T, 3> r(normalize(cross(up, f)));
		//vector<T, 3> u(cross(f, r));
		m_rightVector_W = Set3DVectorComponent(rightVec);
		m_upVector_W = Set3DVectorComponent(upVec);
		m_forwardVector_W = Set3DVectorComponent(forwardVec);

		m_viewMatrix[0] = glm::vec4(m_rightVector_W.x, m_upVector_W.x, m_forwardVector_W.x, 0);
		m_viewMatrix[1] = glm::vec4(m_rightVector_W.y, m_upVector_W.y, m_forwardVector_W.y, 0);
		m_viewMatrix[2] = glm::vec4(m_rightVector_W.z, m_upVector_W.z, m_forwardVector_W.z, 0);
		m_viewMatrix[3] = glm::vec4(-glm::dot(m_rightVector_W, m_position_W), -glm::dot(m_upVector_W, m_position_W), -glm::dot(m_forwardVector_W, m_position_W), 1);

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
		m_position_W+= m_movingSpeed * deltaTime*m_rightVector_W;
		UpdateViewMatrix(Handedness::RightHanded);
	}
	void Camera::UpdateTransform_Y(float  deltaTime)
	{
		m_position_W+= m_movingSpeed * deltaTime*m_upVector_W;
		UpdateViewMatrix(Handedness::RightHanded);
	}
	void Camera::UpdateTransform_Z(float  deltaTime)
	{
		m_position_W+= m_movingSpeed * deltaTime*m_forwardVector_W;
		UpdateViewMatrix(Handedness::RightHanded);
	}

	void Camera::RotateAroundForwardAxis(float degree)
	{
		glm::vec3 axis = glm::vec3(m_forwardVector_W.x, m_forwardVector_W.y, m_forwardVector_W.z);
		glm::mat4 _rotationMatrix = glm::rotate(degree, axis);
		m_upVector_W = _rotationMatrix * m_upVector_W;
		m_rightVector_W = _rotationMatrix * m_rightVector_W;
	}

	void Camera::RotateAroundRightAxis(float degree)
	{
		glm::vec3 axis = glm::vec3(m_rightVector_W.x, m_rightVector_W.y, m_rightVector_W.z);
		glm::mat4 _rotationMatrix = glm::rotate(degree, axis);
		m_forwardVector_W = _rotationMatrix * m_forwardVector_W;
		m_upVector_W = _rotationMatrix * m_upVector_W;
	}

	void Camera::RotateAroundUpAxis(float degree)
	{
		glm::vec3 axis = glm::vec3(m_upVector_W.x, m_upVector_W.y, m_upVector_W.z);
		glm::mat4 _rotationMatrix = glm::rotate(degree, axis);
		m_forwardVector_W = _rotationMatrix * m_forwardVector_W;
		m_rightVector_W = _rotationMatrix * m_rightVector_W;
	}

	Camera::~Camera() {
		vkUnmapMemory(m_device->GetVkDevice(), m_bufferMemory);
		vkDestroyBuffer(m_device->GetVkDevice(), m_buffer, nullptr);
		vkFreeMemory(m_device->GetVkDevice(), m_bufferMemory, nullptr);
	}

	
}