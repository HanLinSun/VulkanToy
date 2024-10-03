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

		const std::vector<std::shared_ptr<ModelGroup>> GetSceneModelGroups();
		const std::vector<ModelGroup*> GetSceneModelGroupsRaw();

		void AddModelGroup(ModelGroup* modelgroup);

		size_t GetModelGroupSize();
		


	private:
		std::vector<std::shared_ptr<ModelGroup>> m_modelGroups;
		std::vector<Material*> m_material;

	};

}