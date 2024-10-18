#pragma once
#include <RendererInclude.h>
#include <SceneStructs.h>
#include <Camera.h>
#include <CameraController.h>
#include <Model.h>

namespace Renderer
{
	class Scene
	{
	public:

		Scene(std::shared_ptr<Camera> Camera);
		Scene();
		~Scene() {};

		const std::vector<std::shared_ptr<ModelGroup>> GetSceneModelGroups();
		const std::vector<ModelGroup*> GetSceneModelGroupsRaw();

		void AddModelGroup(ModelGroup* modelgroup);

		const std::shared_ptr<Camera> GetCamera();
		void SetSceneCamera(std::shared_ptr<Camera> Camera);

		size_t GetModelGroupSize();
		
	private:
		std::vector<std::shared_ptr<ModelGroup>> m_modelGroups;
		std::vector<Material*> m_material;

		std::unique_ptr<CameraController> m_CameraController;
		std::shared_ptr<Camera> m_Camera;
		std::unique_ptr<TextureCubeMap> m_skyboxTexture;

	};

}