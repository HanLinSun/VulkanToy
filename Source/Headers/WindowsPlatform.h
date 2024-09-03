#pragma once
#include "Window.h"
#include <RendererInclude.h>
namespace Renderer
{
	class WindowsWindow :public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		bool WindowResized = false;

		//Window attributes
		inline void SetEventCallback(const EventCallbackFunction& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline virtual void* GetNativeWindow() const { return m_Window; };

		void WindowResizeCallback(GLFWwindow* window, int width, int height);

	private:
		virtual void Init(const WindowProps& prop);
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window;
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;
			bool Resize;
			EventCallbackFunction EventCallback;
		};

		WindowData m_Data;
	};
}