#pragma once

#include "Core.h"
#include "Event/Event.h"
#include "Event/ApplicationEvent.h"
#include "Window.h"
#include "Layer.h"
#include <LayerStack.h>
#include <Vulkan/VulkanBaseRenderer.h>
#include <Input.h>

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

		void PassLayerStackToRenderer();

		inline static Application& Get() { return *s_instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClosed(WindowCloseEvent& e);

	private:
		float m_LastFrameTime = 0.0f;
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<VulkanBaseRenderer> m_baseRenderer;

		bool m_Running = true;
		LayerStack m_LayerStack;

		static Application* s_instance;
	};

	Application* CreateApplication();
}