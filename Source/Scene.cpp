#include "Scene.h"
//This is equal to include real implements
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Headers/modelFileLoader.h"

namespace Renderer
{
	Scene::Scene() {}

	const std::vector<std::shared_ptr<ModelGroup>> Scene::GetSceneModelGroups()
	{
		return m_modelGroups;
	}

	const std::vector<ModelGroup*> Scene::GetSceneModelGroupsRaw()
	{
		std::vector<ModelGroup*> rawPointers;
		for (const auto& modelGroup : m_modelGroups) {
			rawPointers.push_back(modelGroup.get()); // Get raw pointer
		}
		return rawPointers; // Return the vector of raw pointers
	}

	size_t Scene::GetModelGroupSize()
	{
		return m_modelGroups.size();
	}

	void Scene::AddModelGroup(ModelGroup* modelgroup)
	{
		m_modelGroups.push_back(std::unique_ptr<ModelGroup>(modelgroup));
	}
}