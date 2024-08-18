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

		void loadModel(std::string path, std::string texture_path);

		void loadModel(std::string path);

		const std::vector<Polygon> getScenePolygons();
		const std::vector<MeshData> getSceneMeshDatas();
		const MeshData getSceneMeshData(int index);
		const Polygon getScenePolygon(int index);

		void loadPolygonDiffuseTexture(Polygon& m_poly, std::string texture_path);

		void loadMeshTexture(MeshData& m_mesh, std::string texture_path);
		Camera getSceneCamera();

	private:
		std::vector<MeshData> m_meshes;
		std::vector<Polygon> m_polygons;
		Camera m_camera; //This is the main Camera

	};

}