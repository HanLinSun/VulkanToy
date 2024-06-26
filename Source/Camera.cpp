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

	Camera::~Camera(){}

	glm::vec3 Camera::getCameraPosition()
	{
		return m_position;
	}
	
	bool Camera::moving()
	{
		return keyListener.down || keyListener.up || keyListener.left || keyListener.right;
	}

	void Camera::setCameraPosition(glm::vec3 in_position) 
	{
		m_position = in_position;
		updateViewMatrix();
	}

	void Camera::setCameraRotation(glm::vec3 in_rotation)
	{
		m_rotation = in_rotation;
		updateViewMatrix();
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
	void Camera::translateCamera(glm::vec3 translation)
	{
		this->m_position += translation;
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

	void Camera::setPerspectiveMatrix(float fov, float nearPlane, float farPlane)
	{
		glm::mat4 currentMatrix = m_projectionMatrix;
		this->m_farClipPlane = farPlane;
		this->m_nearClipPlane = nearPlane;
		m_projectionMatrix = glm::perspective(glm::radians(fov), m_aspectRatio, m_nearClipPlane, m_farClipPlane);

		if (flipY) {
			m_projectionMatrix[1][1] *= -1.0f;
		}
	}
	

	void Camera::updateViewMatrix()
	{
		m_viewMatrix = glm::lookAt(m_position, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
	}

	void Camera::updateMove()
	{
		if (moving())
		{
			if (keyListener.up)
			{
				m_position += 0.001f* movementSpeed * m_upVector;
			}
			if (keyListener.down)
			{
				m_position += 0.001f * movementSpeed * -m_upVector;
			}
			if (keyListener.right)
			{
				m_position += 0.001f * movementSpeed * m_rightVector;
			}
			if (keyListener.left)
			{
				m_position += 0.001f * movementSpeed * -m_rightVector;
			}
		
			updateViewMatrix();
		}
		
	}
}