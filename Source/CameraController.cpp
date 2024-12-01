#include <CameraController.h>

namespace Renderer
{
	CameraController::CameraController(std::shared_ptr<Camera> m_cam):m_Camera(m_cam),m_moveSpeed(1.f), m_mouseDelta(glm::vec2(0.0f, 0.0f)) {}


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
