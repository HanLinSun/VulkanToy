#include "Camera.h"

namespace Renderer
{
	Camera::Camera() :m_position(glm::vec3(0, 0, 0)), m_rotation(glm::vec3(0, 0, 0)),m_farClipPlane(2000),m_nearClipPlane(0.1),m_fieldOfView(60),
	m_modelMatrix(glm::mat4()),m_viewMatrix(glm::mat4()),m_projectionMatrix(glm::mat4()),m_mvpMatrix(glm::mat4()),
		m_invProjMatrix(glm::mat4()),m_invViewMatrix(glm::mat4()),m_aspectRatio(1.0),m_upVector(glm::vec3(0,1,0)),
		m_refPosition(glm::vec3(0,0,2)),m_rightVector(glm::vec3(1,0,0)),m_lookAt(glm::normalize(m_refPosition - m_position)){}

	Camera::Camera(glm::vec3 init_position) :m_position(init_position), m_rotation(glm::vec3(0, 0, 0)), m_farClipPlane(2000), m_nearClipPlane(0.1), m_fieldOfView(60),
		m_modelMatrix(glm::mat4()), m_viewMatrix(glm::mat4()), m_projectionMatrix(glm::mat4()), m_mvpMatrix(glm::mat4()),
		m_invProjMatrix(glm::mat4()), m_invViewMatrix(glm::mat4()), m_aspectRatio(1.0), m_upVector(glm::vec3(0, 1, 0)),
		m_refPosition(glm::vec3(0, 0, 2)), m_rightVector(glm::vec3(1, 0, 0)), m_lookAt(glm::normalize(m_refPosition - m_position)) {}


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

	// Z is the direction of the lookAt
	// up is y, right is x
	void Camera::translateCamera(glm::vec3 in_direction)
	{
		glm::vec3 m_translateDistance = glm::vec3
		(
			m_rightVector.x * in_direction.x,
			m_upVector.y * in_direction.y,
			m_lookAt.z * in_direction.z
		);

#if USE_VULKAN
		m_translateDistance.y *= -1;
#endif

		m_position += m_translateDistance;
		m_refPosition += m_translateDistance;

		updateViewMatrix();
	}


	void Camera::rotateCamera(glm::vec3 in_rotation)
	{
		m_rotation += in_rotation;
		updateViewMatrix();
	}

	void Camera::setCameraWH(float width, float height)
	{
		m_screenWidth = width;
		m_screenHeight = height;
		m_aspectRatio = width / height;
	}
	
	void Camera::updateAllMatrix()
	{
		m_modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		//view matrix update
		m_viewMatrix = glm::lookAt(m_position, m_refPosition, m_upVector);

#if USE_VULKAN
		//projection matrix for vulkan
	   //In vulkan, NDC is [-1,1], with y axis flipped, which is different from DX
		m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), m_screenWidth / m_screenHeight, m_nearClipPlane, m_farClipPlane);
		m_projectionMatrix[1][1] = -1 * m_projectionMatrix[1][1];
#elif USE_DIRECT_X
		//Direct X has [0,1] NDC coordinates
		// so have different projection matrix, and no need to flip y
		m_projectionMatrix = glm::perspectiveRH(glm::radians(m_fieldOfView), width / height, m_nearClipPlane, m_farClipPlane);
#endif

		m_mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
	}

	void Camera::updateViewMatrix()
	{

	}
}