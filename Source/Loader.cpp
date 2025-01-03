#include "Loader.h"

namespace Renderer
{
	Loader::Loader(std::shared_ptr<Device> device, VkCommandPool commandPool):m_device(device),m_commandPool(commandPool){}
	Loader::~Loader() 
	{
	}
	void Loader::LoadFromSceneFile(Scene* scene, std::string sceneFile)
	{

	}

	void Loader::LoadModel(std::string path, std::string model_folder_path, Scene* scene)
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
				objFileloader->loadFileData(scene, path, model_folder_path);
				std::vector<std::shared_ptr<Material>> mats = scene->GetMaterials();
				std::unique_ptr<ModelGroup> modelGroup = std::make_unique<ModelGroup>();
				for (auto& mesh : objFileloader->GetMeshes())
				{
					Model* model=new Model(m_device.get(), m_commandPool, mesh.m_vertices, mesh.m_indices, mats[mesh.m_materialID],mesh.m_triangles);
					modelGroup->AddModel(model);
				}
				scene->AddModelGroup(std::move(modelGroup));
			}
		}
		
	}

	std::istream& SafeGetline(std::istream& is, std::string& t) {
		t.clear();

		// The characters in the stream are read one-by-one using a std::streambuf.
		// That is faster than reading them one-by-one using the std::istream.
		// Code that uses streambuf this way must be guarded by a sentry object.
		// The sentry object performs various tasks,
		// such as thread synchronization and updating the stream state.

		std::istream::sentry se(is, true);
		std::streambuf* sb = is.rdbuf();

		for (;;) {
			int c = sb->sbumpc();
			switch (c) {
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n')
					sb->sbumpc();
				return is;
			case EOF:
				// Also handle the case when the last line has no line ending
				if (t.empty())
					is.setstate(std::ios::eofbit);
				return is;
			default:
				t += (char)c;
			}
		}
	}
}