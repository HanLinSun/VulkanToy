#pragma once
#include "Core.h"
#include "Event/Event.h"

namespace Renderer
{
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "Vulkan Renderer", uint32_t width = 1600, uint32_t height =720) :Title(title), Width(width), Height(height) {};
	};

	class Window
	{
	public:

		using EventCallbackFunction = std::function<void(Event&)>;
		virtual ~Window() {}
		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		//window attributes
		virtual void SetEventCallback(const EventCallbackFunction& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());

	};
}
