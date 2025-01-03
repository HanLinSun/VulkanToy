#include <WindowsInput.h>
#include <GLFW/glfw3.h>
#include <Application.h>
namespace Renderer
{
	Input* Input::s_Instance = new WindowInput();

	bool WindowInput::IsKeyPressedImpl(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window,keycode);
		return state == GLFW_PRESS || state ==GLFW_REPEAT;
	}

	bool WindowInput::IsMouseButtonPressedImpl(int button)
	{
		auto window =static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}


	bool WindowInput::IsMouseButtonUpImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_RELEASE;
	}

	std::pair<float, float> WindowInput::GetMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		return { (float)xPos, (float)yPos };
	}

	float WindowInput::GetMouseXImpl()
	{
		auto [x, y] = GetMousePositionImpl();
		return x;
	}

	float WindowInput::GetMouseYImpl()
	{
		auto [x, y] = GetMousePositionImpl();
		return y;
	}

	bool WindowInput::IsMouseMoved()
	{
		auto currentMousePosition = GetMousePositionImpl();

		if (currentMousePosition != m_lastMousePosition) {
			m_mouseMoved = true;
			m_lastMousePosition = currentMousePosition;
		}
		else {
			m_mouseMoved = false;
		}

		return m_mouseMoved;
	}

}