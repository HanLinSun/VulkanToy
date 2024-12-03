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
		~Scene();

		const std::vector<ModelGroup*> GetSceneModelGroupsRaw();

		void AddModelGroup(std::unique_ptr<ModelGroup> modelgroup);

		const std::shared_ptr<Camera> GetCamera();
		void SetSceneCamera(std::shared_ptr<Camera> Camera);
		void DestroyVKResources();
		size_t GetModelGroupSize();
		const ModelGroup* GetSceneModelGroup(int idx);
		
	private:
		std::vector<std::unique_ptr<ModelGroup>> m_modelGroups;
	
		std::shared_ptr<Camera> m_Camera;
		std::unique_ptr<TextureCubeMap> m_skyboxTexture;

	};

}