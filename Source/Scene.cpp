#include "Scene.h"
//This is equal to include real implements
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Headers/modelFileLoader.h"

namespace Renderer
{
	Scene::Scene() :m_models() {}

	void Scene::LoadModel(std::string path, std::string texture_path="")
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
		}
		//load texture
		//if (texture_path != "") loadMeshTexture(mesh, texture_path);
		//m_meshes.push_back(mesh);
	}

	void Scene::LoadModel(std::string path)
	{
		//Default only have 1 polygon in one obj file
		MeshData mesh;
		ModelFileLoader* file_loader = nullptr;
		int fileSplit = 0;
		for (int i = path.size() - 1; i > 0; i--)
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
		}
		//Model model(m_device, nullptr, mesh.m_vertices, mesh.m_indices);
		//m_models.push_back(model);
	}


	void Scene::loadMeshTexture(MeshData& m_mesh, std::string texture_path)
	{
		//Texture diffuse_texture;
		//stbi_uc* pixels = stbi_load(texture_path.c_str(), &diffuse_texture.width, &diffuse_texture.height, &diffuse_texture.channels, STBI_rgb_alpha);
		//m_mesh.m_diffuseColorTexture.pixels = pixels;
		//m_mesh.m_diffuseColorTexture.width = diffuse_texture.width;
		//m_mesh.m_diffuseColorTexture.height = diffuse_texture.height;
		//m_mesh.m_diffuseColorTexture.channels = diffuse_texture.channels;
	}


	const std::vector<std::unique_ptr<Model>> Scene::GetSceneModels()
	{
		return m_models;
	}

	const Model* Scene::GetSceneModel(int index)
	{
		return m_models[index].get();
	}

	Camera Scene::GetSceneCamera()
	{
		return m_camera;
	}
}