#include <CameraController.h>
#include <Input.h>

namespace Renderer
{
	glm::vec2 rotationLimit_X = glm::vec2(-2.f * glm::pi<float>(), 2.f * glm::pi<float>());
	glm::vec2 rotationLimit_Y = glm::vec2(-glm::half_pi<float>(), glm::half_pi<float>());

	CameraController::CameraController(std::shared_ptr<Camera> m_cam):m_Camera(m_cam),m_moveSpeed(10.f){}

	glm::vec4 CameraController::GetUpVector()
	{
		return glm::vec4(0, 1, 0,0);
	}

	void CameraController::OnKeyboardEvent()
	{
		m_CameraInputStatus.shouldMove = false;
		m_CameraInputStatus.moveBackward = false;
		m_CameraInputStatus.moveForward = false;
		m_CameraInputStatus.moveLeft = false;
		m_CameraInputStatus.moveRight = false;
		m_CameraInputStatus.moveUp = false;
		m_CameraInputStatus.moveDown = false;

		if (Input::IsKeyPressed(APP_KEY_W))
		{
			m_CameraInputStatus.shouldMove = true;
			m_CameraInputStatus.moveForward = true;
		}

		if (Input::IsKeyPressed(APP_KEY_S))
		{
			m_CameraInputStatus.shouldMove = true;
			m_CameraInputStatus.moveBackward = true;
		}

		if (Input::IsKeyPressed(APP_KEY_A))
		{
			m_CameraInputStatus.shouldMove = true;
			m_CameraInputStatus.moveLeft = true;
		}

		if (Input::IsKeyPressed(APP_KEY_D))
		{
			m_CameraInputStatus.shouldMove = true;
			m_CameraInputStatus.moveRight = true;
		}

		if (Input::IsKeyPressed(APP_KEY_Q))
		{
			m_CameraInputStatus.shouldMove = true;
			m_CameraInputStatus.moveUp = true;
		}

		if (Input::IsKeyPressed(APP_KEY_E))
		{
			m_CameraInputStatus.shouldMove = true;
			m_CameraInputStatus.moveDown = true;
		}
	}

	void CameraController::OnMouseEvent()
	{
		if (Input::IsMouseButtonPressed(0) || Input::IsMouseButtonPressed(1))
		{
			if (Input::IsMouseButtonPressed(0))
			{
				auto pos = Input::GetMousePosition();
				if (!m_CameraInputStatus.isLeftMouseButtonDown)
				{
					m_CameraInputStatus.lastMousePos = glm::vec2(pos.first, pos.second);
					m_CameraInputStatus.isLeftMouseButtonDown = true;
				}
			}

			if (Input::IsMouseButtonPressed(1))
			{
				auto pos = Input::GetMousePosition();
				if (!m_CameraInputStatus.isRightMouseButtonDown)
				{
					m_CameraInputStatus.lastMousePos = glm::vec2(pos.first, pos.second);
					m_CameraInputStatus.isRightMouseButtonDown = true;
				}
			}
		}

		if (Input::IsMouseButtonUp(0) || Input::IsMouseButtonUp(1))
		{
			if (Input::IsMouseButtonUp(0))
			{
				if (m_CameraInputStatus.isLeftMouseButtonDown)
				{
					m_CameraInputStatus.isLeftMouseButtonDown = false;
					m_CameraInputStatus.shouldRotate = false;
				}
			}

			if (Input::IsMouseButtonUp(1))
			{
				if (m_CameraInputStatus.isRightMouseButtonDown)
					m_CameraInputStatus.isRightMouseButtonDown = false;
				//m_cameraInputStatus.shouldRotate = false;
			}
		}

		if (Input::MouseMoved())
		{
			if (m_CameraInputStatus.isLeftMouseButtonDown || m_CameraInputStatus.isRightMouseButtonDown)
			{
				glm::vec2 currentPos = glm::vec2(Input::GetMouseX(), Input::GetMouseY());
				m_CameraInputStatus.mouseDelta = currentPos - m_CameraInputStatus.lastMousePos;
				m_CameraInputStatus.lastMousePos = currentPos;
				m_CameraInputStatus.shouldRotate = true;
			}
			else m_CameraInputStatus.shouldRotate = false;
		}
	}

	void CameraController::Update()
	{
		OnKeyboardEvent();
		OnMouseEvent();

		if (m_Camera != nullptr)
		{
			if (m_CameraInputStatus.shouldRotate)
			{
				glm::vec4 camPos = m_Camera->GetPosition();
				glm::vec4 camTarget = m_Camera->GetLookTarget();
				glm::vec4 camUp = GetUpVector();
				
				
				glm::vec3 viewDir = glm::normalize(Get3DVectorComponent(m_Camera->GetForwardVector()));

				if (m_CameraInputStatus.isLeftMouseButtonDown)
				{

					glm::vec3 sideVector = glm::cross(viewDir, Get3DVectorComponent(camUp));
					float sensitivity = 0.5f;
					glm::vec2 mouseRotation = m_CameraInputStatus.isLeftMouseButtonDown ? m_CameraInputStatus.mouseDelta * sensitivity * Timestep::GetInstance()->GetSeconds() : glm::vec2(0.f);
					glm::vec2 rotation = mouseRotation;

					float x_angle = glm::clamp(rotation.x, rotationLimit_X.x, rotationLimit_Y.y);
					float y_angle = glm::clamp(rotation.y, rotationLimit_Y.x, rotationLimit_Y.y);

					// Rotate around x-axis
					glm::quat qy = glm::angleAxis(y_angle, sideVector);
					glm::mat4 rotY = glm::mat4_cast(qy);

					glm::vec4 viewDir_4 =Set3DVectorComponent(viewDir);
					viewDir_4 = rotY * viewDir_4;

					glm::vec4 upVec_4 = Set3DVectorComponent(camUp);
					upVec_4 = rotY * upVec_4;

					//// Rotate around y-axis
					glm::quat qx = glm::angleAxis(x_angle, Get3DVectorComponent(upVec_4));
					glm::mat4 rotX = glm::mat4_cast(qx);
					viewDir_4 = rotX * viewDir_4;

					glm::vec3 right = glm::cross(Get3DVectorComponent(viewDir_4), Get3DVectorComponent(upVec_4));

					glm::vec4 right_4 = Set3DVectorComponent(right);

					camTarget = camPos + Set3DVectorComponent(viewDir);

					m_Camera->SetLookTarget(camTarget);
					m_Camera->SetForwardVector(viewDir_4);
					m_Camera->SetUpVector(upVec_4);
					m_Camera->SetRightVector(right_4);
					m_dirty = true;
					m_CameraInputStatus.shouldRotate = false;
				}
			}
			//Update Translation
			if (m_CameraInputStatus.shouldMove)
			{
				glm::vec3 moveDir(0);
				moveDir.z += m_CameraInputStatus.moveForward ? 1 : 0;
				moveDir.z += m_CameraInputStatus.moveBackward ? -1 : 0;
				moveDir.x += m_CameraInputStatus.moveLeft ? -1 : 0;
				moveDir.x += m_CameraInputStatus.moveRight ? 1 : 0;
				moveDir.y += m_CameraInputStatus.moveUp ? 1 : 0;
				moveDir.y += m_CameraInputStatus.moveDown ? -1 : 0;


				glm::vec4 camPos = m_Camera->GetPosition();
				glm::vec4 camTarget = m_Camera->GetLookTarget();
				glm::vec3 camUp = m_Camera->GetUpVector();
				glm::vec3 viewDir = glm::normalize(Get3DVectorComponent(camTarget) - Get3DVectorComponent(camPos));
				glm::vec3 sideVector = glm::cross(viewDir, camUp);

				camPos += m_moveSpeed * Timestep::GetInstance()->GetSeconds() * moveDir.x * Set3DVectorComponent(sideVector);
				camPos += m_moveSpeed * Timestep::GetInstance()->GetSeconds() * moveDir.y * Set3DVectorComponent(camUp);
				camPos += m_moveSpeed * Timestep::GetInstance()->GetSeconds() * moveDir.z * Set3DVectorComponent(viewDir);

				camTarget = camPos + Set3DVectorComponent(viewDir);
				m_Camera->SetPosition(camPos);
				m_Camera->SetLookTarget(camTarget);
				m_dirty = true;
			}

			if (m_dirty)
			{
				m_Camera->UpdateViewMatrixFromLookAt(Handedness::RightHanded,glm::vec3(0,1,0));
				m_Camera->UpdateBufferMemory();
				m_dirty = false;
			}

		}

	}

}
