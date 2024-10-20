#pragma once
#include <vector>
#include <fstream>
#include <string>
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include "Scene.h"
#include <memory>


namespace Renderer
{
	class FileLoader
	{
	public:

		FileLoader();
		virtual ~FileLoader() {};

		virtual void loadFileData(std::string modelFilePath, std::string modelFolderPath);
	};


	class ObjFileLoader : public FileLoader
	{
	public:
		ObjFileLoader() = delete;
		ObjFileLoader(std::shared_ptr<Device> device);
		virtual ~ObjFileLoader();

		void loadFileData(std::string modelFilePath, std::string modelFolderPath) override;
		std::vector<MeshData> GetMeshes();
		std::vector<std::shared_ptr<Material>> GetMaterials();
		std::shared_ptr<Device> GetDevice();

	protected:
		std::shared_ptr<Device> m_device;
		std::vector<MeshData> m_meshes;
		std::vector<std::shared_ptr<Material>> m_materials;
	};

	class GLTFFileLoader :public FileLoader
	{
	public:
		GLTFFileLoader();
		virtual ~GLTFFileLoader();

		void loadFileData(std::string modelFilePath,std::string modelFolderPath) override;

		std::vector<GLTFMeshData> m_meshes;
	};
}