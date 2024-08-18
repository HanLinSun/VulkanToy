#pragma once
#include "Application.h"
#include "Layer.h"
#include "ImGUILayer.h"
namespace Renderer
{
	class RenderApplication : public Application
	{
	public:
		RenderApplication()
		{
			PushOverLayer(new ImGuiLayer());
			PassLayerStackToRenderer();
		}
		~RenderApplication() {};
	};
}

Renderer::Application* CreateApplication()
{
	return new Renderer::RenderApplication();
}