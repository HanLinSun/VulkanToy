#include "Camera.h"
#include <glm/gtx/transform.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/common.hpp>
#include <Input.h>
namespace Renderer
{
	glm::vec2 rotationXLimit = glm::vec2(-2.f*glm::pi<float>(), 2.f * glm::pi<float>());
	glm::vec2 rotationYLimit = glm::vec2(-glm::half_pi<float>(), glm::half_pi<float>());

	Camera::Camera(Device* device, float aspectRatio) :m_device(device)
	{
		m_position_W = glm::vec4(0.0f, 0.0f, 0.0f,1.0f);
		m_upVector_W = glm::vec4(0.0f, 1.0f, 0.0f,0.0f);

		m_lookTarget_W = glm::vec4(0.0f,0.0f, -1.0f, 1.0f);

		m_forwardVector_W = glm::normalize(m_lookTarget_W -m_position_W);

		//m_forwardVector_W = glm::vec4(0.0f, 0.0f, -1.0f,0.0f);
		m_aperture = 0.02f;
		m_focalDistance = 0.1f;

		glm::vec3 m_rightVec_3 = glm::cross(Get3DVectorComponent(m_forwardVector_W), Get3DVectorComponent(m_upVector_W));
		m_rightVector_W = Set3DVectorComponent(m_rightVec_3);

		m_aspectRatio = aspectRatio;
		m_nearClipPlane = 0.1f;
		m_farClipPlane = 5500.0f;
		m_fov = 60.0f;

		UpdateViewMatrixFromLookAt(Handedness::RightHanded, Get3DVectorComponent(m_upVector_W));
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

	glm::mat4 Camera::GetProjectionMatrix() const
	{
		return m_projectionMatrix;
	}

	glm::mat4 Camera::GetViewmatrix() const
	{
		return m_viewMatrix;
	}

	void Camera::UpdateViewMatrix(Handedness hand)
	{
		m_forwardVector_W = (hand == Handedness::RightHanded) ? glm::normalize(m_position_W - m_lookTarget_W) : glm::normalize(m_lookTarget_W - m_position_W);
	
		m_viewMatrix[0] =glm::vec4(m_rightVector_W.x, m_upVector_W.x, m_forwardVector_W.x, 0);
		m_viewMatrix[1] = glm::vec4(m_rightVector_W.y, m_upVector_W.y, m_forwardVector_W.y, 0);
		m_viewMatrix[2] = glm::vec4(m_rightVector_W.z, m_upVector_W.z, m_forwardVector_W.z, 0);
		m_viewMatrix[3] = glm::vec4(-glm::dot(m_rightVector_W,m_position_W), -glm::dot(m_upVector_W, m_position_W), -glm::dot(m_forwardVector_W, m_position_W),1);

	}

	void Camera::UpdateViewMatrixFromLookAt(Handedness hand,glm::vec3 upVec)
	{

		glm::vec3 forwardVec = Get3DVectorComponent(m_forwardVector_W);

		glm::vec3 rightVec = glm::normalize(glm::cross(forwardVec,upVec));

		glm::vec3 upVec_1 = glm::normalize(glm::cross(rightVec,forwardVec));

		//vector<T, 3> r(normalize(cross(up, f)));
		//vector<T, 3> u(cross(f, r));
		glm::vec3 position = Get3DVectorComponent(m_position_W);

		m_viewMatrix[0] = glm::vec4(rightVec.x, upVec_1.x, forwardVec.x, 0);
		m_viewMatrix[1] = glm::vec4(rightVec.y, upVec_1.y, forwardVec.y, 0);
		m_viewMatrix[2] = glm::vec4(rightVec.z, upVec_1.z, forwardVec.z, 0);
		m_viewMatrix[3] = glm::vec4(-glm::dot(rightVec, position), -glm::dot(upVec_1, position), -glm::dot(forwardVec, position), 1);
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

	void Camera::DestroyVKResources()
	{
		vkUnmapMemory(m_device->GetVkDevice(), m_bufferMemory);
		vkDestroyBuffer(m_device->GetVkDevice(), m_buffer, nullptr);
		vkFreeMemory(m_device->GetVkDevice(), m_bufferMemory, nullptr);
	}

	Camera::~Camera() {}

	void Camera::SetUpVector(glm::vec4 upVec)
	{
		m_upVector_W = upVec;
	}
	void Camera::SetLookTarget(glm::vec4 lookTarget)
	{
		m_lookTarget_W = lookTarget;
	}
	void Camera::SetRightVector(glm::vec4 rightVec)
	{
		m_rightVector_W = rightVec;
	}
	void Camera::SetForwardVector(glm::vec4 forward)
	{
		m_forwardVector_W = forward;
	}
	void Camera::SetPosition(glm::vec4 position)
	{
		m_position_W = position;
	}

	glm::vec4 Camera::GetUpVector() const
	{
		return m_upVector_W;
	}
	glm::vec4 Camera::GetLookTarget() const
	{
		return m_lookTarget_W;
	}

	glm::vec4 Camera::GetRightVector() const
	{
		return m_rightVector_W;
	}
	glm::vec4 Camera::GetForwardVector() const
	{
		return m_forwardVector_W;
	}
	glm::vec4 Camera::GetPosition() const
	{
		return m_position_W;
	}

	float Camera::GetAspectRatio() const
	{
		return m_aspectRatio;
	}

	float Camera::GetFOV() const
	{
		return m_fov;
	}

	glm::float32_t Camera::GetAperture() const
	{
		return m_aperture;
	}

	void Camera::SetAperture(glm::float32_t& aperture)
	{
		m_aperture = aperture;
	}
	glm::float32_t Camera::GetFocalDistance() const
	{
		return m_focalDistance;
	}
	void Camera::SetFocalDistance(glm::float32_t& focalDistance)
	{
		m_focalDistance = focalDistance;
	}


	
}