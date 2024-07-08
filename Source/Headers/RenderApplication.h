#pragma once
#include "IncludeUtils.h"
#include "ImGUILayer.h"
namespace Renderer
{
	class RenderApplication : public Application
	{
	public:
		RenderApplication()
		{
			PushOverLayer(new ImGuiLayer());
		}
		~RenderApplication() {};
	};

	Application* CreateApplication()
	{
		return new RenderApplication();
	}
}