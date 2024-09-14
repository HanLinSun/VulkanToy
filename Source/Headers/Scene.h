#pragma once
#include <RendererInclude.h>
#include "SceneStructs.h"
#include "Camera.h"

namespace Renderer
{
	class Scene
	{
	public:

		Scene();
		~Scene() {};

		void LoadModel(std::string path, std::string texture_path);

		void LoadModel(std::string path);

		const std::vector<MeshData> getSceneMeshDatas();
		const MeshData getSceneMeshData(int index);
		void loadMeshTexture(MeshData& m_mesh, std::string texture_path);
		Camera getSceneCamera();

	private:
		std::vector<MeshData> m_meshes;
		Camera m_camera; //This is the main Camera

	};

}