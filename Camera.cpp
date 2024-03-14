#include "Camera.h"

namespace vulkanRenderer
{
	Camera::Camera() :m_position(glm::vec3(0, 0, 0)), m_rotation(0, 0, 0),m_lookAt(glm::vec3(0,0,-1)),m_farClipPlane(2000),m_nearClipPlane(0.2),m_fieldOfView(60),
	m_modelMatrix(glm::mat4()),m_viewMatrix(glm::mat4()),m_projectionMatrix(glm::mat4()),m_mvpMatrix(glm::mat4()),
		m_invProjMatrix(glm::mat4()),m_invViewMatrix(glm::mat4()),m_aspectRatio(1.0),m_upVector(glm::vec3(0,0,1)){}

	Camera::Camera(glm::vec3 init_position) :m_position(init_position), m_rotation(0, 0, 0), m_lookAt(glm::vec3(0, 0, -1)), m_farClipPlane(2000), m_nearClipPlane(0.2), m_fieldOfView(60),
		m_modelMatrix(glm::mat4()), m_viewMatrix(glm::mat4()), m_projectionMatrix(glm::mat4()), m_mvpMatrix(glm::mat4()),
		m_invProjMatrix(glm::mat4()), m_invViewMatrix(glm::mat4()), m_aspectRatio(1.0), m_upVector(glm::vec3(0, 0, 1)) {}


	glm::vec3 Camera::getCameraPosition()
	{
		return m_position;
	}

	void Camera::updateCameraPosition(glm::vec3 in_position) 
	{
		m_position = in_position;
	}

	void Camera::updateCameraRotation(glm::vec3 in_rotation)
	{
		m_rotation = in_rotation;
	}

	glm::mat4 Camera::getProjectionMatrix()
	{
		return m_projectionMatrix;
	}

	glm::mat4 Camera::getViewMatrix()
	{
		return m_viewMatrix;
	}

	glm::mat4 Camera::getMVPMatrix()
	{
		return m_mvpMatrix;
	}

	//In vulkan, NDC is [-1,1], with y axis flipped
	void Camera::updateMatrix(float width, float height)
	{
		m_modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		//view matrix update
		m_viewMatrix = glm::lookAt(m_position, glm::vec3(0, 0, 0), m_upVector);

		//projection matrix
		m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), width / height, m_nearClipPlane, m_farClipPlane);
		m_projectionMatrix[1][1] = -1 * m_projectionMatrix[1][1];
		//

		m_mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
	}
}