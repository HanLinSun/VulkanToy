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

	class Camera;
	struct InputStatus
	{
		glm::vec2 lastMousePos;
		glm::vec2 mouseDelta;
		bool isLeftMouseButtonDown;
		bool isRightMouseButtonDown;
		bool shouldRotate;

		bool shouldMove;
		bool moveForward;
		bool moveBackward;
		bool moveLeft;
		bool moveRight;
		bool moveUp;
		bool moveDown;
	};

	class CameraController
	{
		//By default ONLY implement FPS Camera controller
	public:
		CameraController(std::shared_ptr<Camera> m_cam);
		~CameraController() = default;
		glm::vec4 GetUpVector();

		void OnKeyboardEvent();
		void OnMouseEvent();
		void Update();

		InputStatus m_CameraInputStatus;
	protected:
		std::shared_ptr<Camera> m_Camera;


		float m_moveSpeed;
		bool m_mouseMoved;
		bool m_dirty;
	};
}
