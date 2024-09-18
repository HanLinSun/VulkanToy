#include "Loader.h"

namespace Renderer
{
	Loader::Loader(Device* device, VkCommandPool commandPool):m_device(device),m_commandPool(commandPool){}
	Loader::~Loader() 
	{
		vkDestroyCommandPool(m_device->GetVkDevice(),m_commandPool,nullptr);
	}
	void Loader::LoadModel(std::string path, std::string texture_path, std::vector<Model*>& models)
	{
		MeshData mesh;
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
			ObjFileLoader* objFileloader = new ObjFileLoader(m_device);
			if (objFileloader != nullptr)
			{
				objFileloader->loadFileData(path);
				for (auto& mesh : objFileloader->GetMeshes())
				{
					Model* model=new Model(m_device, m_commandPool, mesh.m_vertices, mesh.m_indices);
					models.push_back(model);
				}
			}
		}
		
	}
	void Loader::LoadModel(std::string path, std::vector<Model*>& models)
	{
		MeshData mesh;
		FileLoader* file_loader = new FileLoader();
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
			ObjFileLoader* objFileloader = dynamic_cast<ObjFileLoader*>(file_loader);
			if (objFileloader != nullptr)
			{
				objFileloader->loadFileData(path);
				for (auto& mesh : objFileloader->GetMeshes())
				{
					Model* model = new Model(m_device, m_commandPool, mesh.m_vertices, mesh.m_indices);
					models.push_back(model);
				}
			}
		}
	}

}