#pragma once
#include <RendererInclude.h>
#include "SceneStructs.h"
#include "Camera.h"
#include "Model.h"

namespace Renderer
{
	class Scene
	{
	public:

		Scene();
		~Scene() {};

		const std::vector<Model*> GetSceneModels();
		const Model* GetSceneModel(int index);

		void AddModels(std::vector<Model*>& _models);
		void AddModel(Model* _model);

		Camera GetSceneCamera();

	private:
		std::vector<Model*> m_models;
		Camera m_camera; //This is the main Camera

	};

}