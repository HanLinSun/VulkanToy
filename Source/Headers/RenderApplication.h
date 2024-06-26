#pragma once
#include "IncludeUtils.h"
namespace Renderer
{
	class TestLayer: public Layer
	{
		TestLayer() :Layer("Example")
		{

		}

		void OnUpdate() override
		{

		}

		void OnEvent(Event& e) override
		{

		}

	};

	class RenderApplication : public Application
	{
	public:
		RenderApplication()
		{
			PushLayer(new TestLayer());
		}
		~RenderApplication();
	};

	Application* CreateApplication()
	{
		return new RenderApplication();
	}
}