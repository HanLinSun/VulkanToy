#pragma once

#include "Core.h"
#include "Event/Event.h"
#include "Event/ApplicationEvent.h"
#include "Window.h"
#include "Layer.h"
#include "LayerStack.h"

namespace Renderer
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverLayer(Layer* layer);

	private:
		bool OnWindowClosed(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;

		LayerStack m_LayerStack;
	};

	Application* CreateApplication();
}