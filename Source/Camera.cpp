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
		m_position_W = glm::vec4(0.0f, 30.0f, 0.0f,1.0f);
		m_upVector_W = glm::vec4(0.0f, 1.0f, 0.0f,0.0f);
		
		m_lookTarget_W = glm::vec4(100.f, 30.0f, 0.0f, 1.0f);

		m_forwardVector_W = glm::normalize(m_lookTarget_W - m_position_W);

		//m_forwardVector_W = glm::vec4(0.0f, 0.0f, -1.0f,0.0f);

		glm::vec3 m_rightVec_3 = glm::cross(Get3DVectorComponent(m_forwardVector_W), Get3DVectorComponent(m_upVector_W));
		m_rightVector_W = Set3DVectorComponent(m_rightVec_3);

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
		//m_forwardVector_W = (hand == Handedness::RightHanded) ? glm::normalize(m_position_W - m_lookTarget_W) : glm::normalize(m_lookTarget_W - m_position_W);
		//m_viewMatrix = rotationMatrix * translateMatrix;
		m_viewMatrix[0] =glm::vec4(m_rightVector_W.x, m_upVector_W.x, m_forwardVector_W.x, 0);
		m_viewMatrix[1] = glm::vec4(m_rightVector_W.y, m_upVector_W.y, m_forwardVector_W.y, 0);
		m_viewMatrix[2] = glm::vec4(m_rightVector_W.z, m_upVector_W.z, m_forwardVector_W.z, 0);
		m_viewMatrix[3] = glm::vec4(-glm::dot(m_rightVector_W,m_position_W), -glm::dot(m_upVector_W, m_position_W), -glm::dot(m_forwardVector_W, m_position_W),1);
		//m_viewMatrix = glm::lookAt(Get3DVectorComponent(m_position_W), Get3DVectorComponent(m_lookTarget_W), Get3DVectorComponent(m_upVector_W));

	}

	void Camera::UpdateViewMatrixFromLookAt(Handedness hand)
	{
		m_forwardVector_W = (hand == Handedness::RightHanded) ? glm::normalize(m_lookTarget_W-m_position_W) : glm::normalize(m_position_W - m_lookTarget_W);

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

	void Camera::Update()
	{
		HandleMouseInputEvent();
    	HandleKeyboardInputEvent();

		if (m_cameraInputStatus.shouldRotate)
		{
			glm::vec3 viewDir = glm::normalize(Get3DVectorComponent(m_lookTarget_W - m_position_W));
			if (m_cameraInputStatus.isLeftMouseButtonDown)
			{
				glm::vec3 sideVector = glm::cross(viewDir, Get3DVectorComponent(m_upVector_W));

				float sensitivity =0.5f;
				glm::vec2 mouseRotation = m_cameraInputStatus.isLeftMouseButtonDown ? m_cameraInputStatus.mouseDelta * sensitivity* Timestep::GetInstance()->GetSeconds() : glm::vec2(0.f);
				glm::vec2 rotation = mouseRotation;

				float x_angle = glm::clamp(rotation.x, rotationXLimit.x, rotationXLimit.y);
				float y_angle = glm::clamp(rotation.y, rotationYLimit.x, rotationYLimit.y);

				// Rotate around x-axis
				glm::quat qy = glm::angleAxis(y_angle, sideVector);
				glm::mat4 rotY = glm::mat4_cast(qy);
				m_forwardVector_W = rotY * m_forwardVector_W;
				m_upVector_W = rotY * m_upVector_W;


				// Rotate around y-axis
				glm::quat qx = glm::angleAxis(x_angle, Get3DVectorComponent(m_upVector_W));
				glm::mat4 rotX = glm::mat4_cast(qx);
				m_forwardVector_W = rotX * m_forwardVector_W;

				glm::vec3 right = glm::cross(Get3DVectorComponent(m_forwardVector_W), Get3DVectorComponent(m_upVector_W));
				m_rightVector_W = Set3DVectorComponent(right);

				m_lookTarget_W = m_position_W + m_forwardVector_W;

				m_dirty = true;
				m_cameraInputStatus.shouldRotate = false;
			}
		}

		//Update Translation
		if (m_cameraInputStatus.shouldMove)
		{
			glm::vec3 moveDir(0);
			moveDir.z += m_cameraInputStatus.moveForward ? 1 : 0;
			moveDir.z += m_cameraInputStatus.moveBackward ? -1 : 0;
			moveDir.x += m_cameraInputStatus.moveLeft ? -1 : 0;
			moveDir.x += m_cameraInputStatus.moveRight ? 1 : 0;
			moveDir.y += m_cameraInputStatus.moveUp ? 1 : 0;
			moveDir.y += m_cameraInputStatus.moveDown ? -1 : 0;

			m_position_W += m_movingSpeed * Timestep::GetInstance()->GetSeconds()* moveDir.x * m_rightVector_W;
			m_position_W += m_movingSpeed * Timestep::GetInstance()->GetSeconds() * moveDir.y * m_upVector_W;
			m_position_W += m_movingSpeed * Timestep::GetInstance()->GetSeconds() * moveDir.z * m_forwardVector_W;

			
			m_lookTarget_W = m_position_W + m_forwardVector_W;
			m_dirty = true;
		}
		

		if (m_dirty)
		{
			UpdateViewMatrix(Handedness::RightHanded);
			UpdateBufferMemory();
			m_dirty = false;
		}

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

	void Camera::HandleMouseInputEvent()
	{
		if (Input::IsMouseButtonPressed(0) || Input::IsMouseButtonPressed(1))
		{
			if (Input::IsMouseButtonPressed(0))
			{
				auto pos = Input::GetMousePosition();
				if (!m_cameraInputStatus.isLeftMouseButtonDown)
				{
					m_cameraInputStatus.lastMousePos = glm::vec2(pos.first, pos.second);
					m_cameraInputStatus.isLeftMouseButtonDown = true;
				}

			}
			
			if (Input::IsMouseButtonPressed(1))
			{
				auto pos = Input::GetMousePosition();
				if (!m_cameraInputStatus.isRightMouseButtonDown)
				{
					m_cameraInputStatus.lastMousePos = glm::vec2(pos.first, pos.second);
					m_cameraInputStatus.isRightMouseButtonDown = true;
				}
			}
		}

		if (Input::IsMouseButtonUp(0) || Input::IsMouseButtonUp(1))
		{
			if (Input::IsMouseButtonUp(0))
			{
				if (m_cameraInputStatus.isLeftMouseButtonDown)
				{
					m_cameraInputStatus.isLeftMouseButtonDown = false;
					m_cameraInputStatus.shouldRotate = false;
				}
			}

			if (Input::IsMouseButtonUp(1))
			{
				if(m_cameraInputStatus.isRightMouseButtonDown)
				m_cameraInputStatus.isRightMouseButtonDown = false;
				//m_cameraInputStatus.shouldRotate = false;
			}
		}

		if (Input::MouseMoved())
		{
			if (m_cameraInputStatus.isLeftMouseButtonDown || m_cameraInputStatus.isRightMouseButtonDown)
			{
				glm::vec2 currentPos = glm::vec2(Input::GetMouseX(), Input::GetMouseY());
				m_cameraInputStatus.mouseDelta = currentPos - m_cameraInputStatus.lastMousePos;
				m_cameraInputStatus.lastMousePos = currentPos;
				m_cameraInputStatus.shouldRotate = true;
			}
			else m_cameraInputStatus.shouldRotate = false;
		}
	}

	void Camera::HandleKeyboardInputEvent()
	{
		m_cameraInputStatus.shouldMove = false;
		m_cameraInputStatus.moveBackward = false;
		m_cameraInputStatus.moveForward = false;
		m_cameraInputStatus.moveLeft = false;
		m_cameraInputStatus.moveRight = false;
		m_cameraInputStatus.moveUp = false;
		m_cameraInputStatus.moveDown = false;

		if (Input::IsKeyPressed(APP_KEY_W))
		{
			m_cameraInputStatus.shouldMove = true;
			m_cameraInputStatus.moveForward = true;
		}

		if (Input::IsKeyPressed(APP_KEY_S))
		{
			m_cameraInputStatus.shouldMove = true;
			m_cameraInputStatus.moveBackward = true;
		}

		if (Input::IsKeyPressed(APP_KEY_A))
		{
			m_cameraInputStatus.shouldMove = true;
			m_cameraInputStatus.moveLeft = true;
		}

		if (Input::IsKeyPressed(APP_KEY_D))
		{
			m_cameraInputStatus.shouldMove = true;
			m_cameraInputStatus.moveRight = true;
		}

		if (Input::IsKeyPressed(APP_KEY_Q))
		{
			m_cameraInputStatus.shouldMove = true;
			m_cameraInputStatus.moveUp = true;
		}

		if (Input::IsKeyPressed(APP_KEY_E))
		{
			m_cameraInputStatus.shouldMove = true;
			m_cameraInputStatus.moveDown = true;
		}
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


	void Camera::DestroyVKResources()
	{
		vkUnmapMemory(m_device->GetVkDevice(), m_bufferMemory);
		vkDestroyBuffer(m_device->GetVkDevice(), m_buffer, nullptr);
		vkFreeMemory(m_device->GetVkDevice(), m_bufferMemory, nullptr);
	}

	Camera::~Camera() 
	{
	
	}

	
}