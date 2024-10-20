#pragma once
#include <Camera.h>
#include <memory>
#include <RendererInclude.h>

namespace Renderer
{
	enum CameraControllerType
	{
		FirstPerson,
		Orbit, 
		//SixDof,
	};

	class CameraController
	{
		//By default ONLY implement FPS Camera controller
	public:
		CameraController(std::shared_ptr<Camera> m_cam);

		void SetMovementSpeed(float movementSpeed);
		float GetMovementSpeed();
		void GetUpDirection();

		void OnKeyboardEvent();
		void OnMouseEvent();

	protected:
		std::shared_ptr<Camera> m_Camera;
		float m_moveSpeed;
		bool m_mouseMoved;

		float r, theta, phi;
		glm::vec2 m_mouseDelta;
	};
}
