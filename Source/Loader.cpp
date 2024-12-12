#include "Loader.h"

namespace Renderer
{
	Loader::Loader(std::shared_ptr<Device> device, VkCommandPool commandPool):m_device(device),m_commandPool(commandPool){}
	Loader::~Loader() 
	{
	}
	void Loader::LoadModel(std::string path, std::string model_folder_path, ModelGroup* modelGroup)
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
				objFileloader->loadFileData(path, model_folder_path);
				std::vector<std::shared_ptr<Material>> mats = objFileloader->GetMaterials();
				for (auto& mesh : objFileloader->GetMeshes())
				{
					Model* model=new Model(m_device.get(), m_commandPool, mesh.m_vertices, mesh.m_indices, mats[mesh.m_materialID],mesh.m_triangles);
					modelGroup->AddModel(model);
				}

				for (auto& ptr : mats)
				{
					modelGroup->AddMaterial(ptr);
				}
			}
		}
		
	}

}