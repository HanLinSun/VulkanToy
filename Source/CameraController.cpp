#include <CameraController.h>

namespace Renderer
{
	CameraController::CameraController(std::shared_ptr<Camera> m_cam):m_Camera(m_cam),m_moveSpeed(100), m_mouseDelta(glm::vec2(0.0f, 0.0f)) {}

	void CameraController::SetMovementSpeed(float movementSpeed)
	{
		m_moveSpeed = movementSpeed;
	}

	float CameraController::GetMovementSpeed()
	{
		return m_moveSpeed;
	}

	void CameraController::GetUpDirection()
	{

	}

	void CameraController::OnKeyboardEvent()
	{

	}

	void CameraController::OnMouseEvent()
	{

	}

}
