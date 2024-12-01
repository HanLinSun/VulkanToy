#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <windows.h>
#include <SceneStructs.h>
#include <Core.h>

namespace Renderer
{



	class Input
	{
	public:
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static bool IsMouseButtonUp(int button) { return s_Instance->IsMouseButtonUpImpl(button); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
		inline static bool MouseMoved() { return s_Instance->IsMouseMoved(); }
		

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual bool IsMouseButtonUpImpl(int button) = 0;
		virtual bool IsMouseMoved() = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

		std::pair<float, float> m_lastMousePosition = { 0.0f, 0.0f };
		bool m_mouseMoved = false;
	private:
		static Input* s_Instance;
	};
}
