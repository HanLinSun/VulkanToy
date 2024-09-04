#include "Application.h"
#include "Event/ApplicationEvent.h"
#include "Headers/Event/Event.h"
#include "Headers/WindowsPlatform.h"
#include "Log.h"
namespace Renderer
{
	Application* Application::s_instance = nullptr;

#define BIND_EVENT_FUNCTION(x) std::bind(&x,this,std::placeholders::_1)

	Application::Application()
	{
		s_instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FUNCTION(Application::OnEvent));
		m_baseRenderer = std::unique_ptr<VulkanBaseRenderer>(new VulkanBaseRenderer(m_Window.get()));
	 }

	Application::~Application(){}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FUNCTION(Application::OnWindowClosed));
		LOG_CORE_INFO("{0}",e.ToString());
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
			{
				break;
			}
		}
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

	void Application::PassLayerStackToRenderer()
	{
		m_baseRenderer->SetLayerStack(&m_LayerStack);
	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		if (e.InInCategory(EventCategoryApplication))
		{
			LOG_CORE_TRACE(e.ToString());
		}

		if (e.InInCategory(EventCategoryInput))
		{
			LOG_CORE_TRACE(e.ToString());
		}

		m_baseRenderer->InitVulkan();
		m_baseRenderer->InitGUILayerAttribute();

		while (m_Running)
		{
			float time = (float)glfwGetTime(); //Maybe in the future it will become platform based
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			m_Window->OnUpdate();
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate();
			}
			m_baseRenderer->Run();
		}
		m_baseRenderer->Destroy();
	}

}