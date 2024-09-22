#include "Scene.h"
//This is equal to include real implements
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Headers/modelFileLoader.h"

namespace Renderer
{
	Scene::Scene() {}

	const std::vector<Model*> Scene::GetSceneModels()
	{
		return m_models;
	}

	const Model* Scene::GetSceneModel(int index)
	{
		return m_models[index];
	}


	void Scene::AddModels(std::vector<Model*>& _models)
	{
		if (m_models.size() == 0)
		{
			m_models = _models;
		}
		else
		{
			for (auto& model : _models)
			{
				m_models.push_back(model);
			}
		}
	}
	void Scene::AddModel(Model* _model)
	{
		m_models.push_back(_model);
	}

}