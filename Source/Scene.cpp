#include "Scene.h"
//This is equal to include real implements
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Headers/modelFileLoader.h"

namespace Renderer
{
	Scene::Scene() :m_polygons() {}

	void Scene::loadModel(std::string path, std::string texture_path)
	{
		//Default only have 1 polygon in one obj file
		MeshData mesh;
		ModelFileLoader* file_loader = nullptr;
		int fileSplit = 0;
		for (int i = path.size()-1; i > 0; i--)
		{
			if (path[i] == '.')
			{
				fileSplit = i;
				break;
			}
		}

		std::string fileType = path.substr(fileSplit, path.size() - fileSplit);
		if (fileType == ".obj")
		{
			file_loader = new ObjFileLoader;
			file_loader->loadFileData(path);
			file_loader->loadDataToMesh(mesh);
		}
		//load texture
		loadMeshTexture(mesh, texture_path);
		m_meshes.push_back(mesh);
	}

	void Scene::loadPolygonDiffuseTexture(Polygon& in_polygon, std::string texture_path)
	{
		Texture diffuse_texture;
		stbi_uc* pixels = stbi_load(texture_path.c_str(), &diffuse_texture.width, &diffuse_texture.height, &diffuse_texture.channels, STBI_rgb_alpha);
		in_polygon.m_diffuseColorTexture.pixels = pixels;
		in_polygon.m_diffuseColorTexture.width = diffuse_texture.width;
		in_polygon.m_diffuseColorTexture.height = diffuse_texture.height;
		in_polygon.m_diffuseColorTexture.channels = diffuse_texture.channels;
	}

	void Scene::loadMeshTexture(MeshData& m_mesh, std::string texture_path)
	{
		Texture diffuse_texture;
		stbi_uc* pixels = stbi_load(texture_path.c_str(), &diffuse_texture.width, &diffuse_texture.height, &diffuse_texture.channels, STBI_rgb_alpha);
		m_mesh.m_diffuseColorTexture.pixels = pixels;
		m_mesh.m_diffuseColorTexture.width = diffuse_texture.width;
		m_mesh.m_diffuseColorTexture.height = diffuse_texture.height;
		m_mesh.m_diffuseColorTexture.channels = diffuse_texture.channels;
	}

	const std::vector<Polygon> Scene::getScenePolygons()
	{
		return m_polygons;
	}

	const std::vector<MeshData> Scene::getSceneMeshDatas()
	{
		return m_meshes;
	}

	const MeshData Scene::getSceneMeshData(int index)
	{
		return m_meshes[index];
	}
	const Polygon Scene::getScenePolygon(int index)
	{
		return m_polygons[index];
	}

	Camera Scene::getSceneCamera()
	{
		return m_camera;
	}
}