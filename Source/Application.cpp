#include "Application.h"
#include "Event/ApplicationEvent.h"
#include "Headers/Event/Event.h"
#include "Headers/WindowsPlatform.h"
namespace Renderer
{
	Application* Application::s_instance = nullptr;
#define BIND_EVENT_FUNCTION(x) std::bind(&x,this,std::placeholders::_1)

	Application::Application()
	{
		s_instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FUNCTION(Application::OnEvent));
	 }

	Application::~Application(){}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FUNCTION(Application::OnWindowClosed));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
			{
				break;
			}
		}

		std::cout << " OnEvent triggered" << std::endl;
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e) 
	{
		m_Running = false;
		return true;
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverLayer(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		if (e.InInCategory(EventCategoryApplication))
		{
			std::cout << "Log: application event triggered" << std::endl;
		}

		if (e.InInCategory(EventCategoryInput))
		{
			std::cout << "Log: Input event triggered" << std::endl;
		}

		while (m_Running)
		{
			//m_renderer->nextFrame();
			m_Window->OnUpdate();
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate();
			}

		}
	}

}