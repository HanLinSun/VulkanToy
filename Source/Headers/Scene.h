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

		Scene() = delete;

		Scene(Device* device):m_device(device){}
		~Scene() {};

		void LoadModel(std::string path, std::string texture_path);

		void LoadModel(std::string path);

		const std::vector<std::unique_ptr<Model>> GetSceneModels();
		const Model* GetSceneModel(int index);

		void AddModels(std::vector<Model>& _models);
		void AddModel(Model& _model);

		void loadMeshTexture(MeshData& m_mesh, std::string texture_path);
		Camera GetSceneCamera();

	private:
		Device* m_device;
		std::vector<std::unique_ptr<Model>> m_models;
		Camera m_camera; //This is the main Camera

	};

}