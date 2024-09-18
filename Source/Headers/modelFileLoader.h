#pragma once
#include <vector>
#include <fstream>
#include <string>
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include "Scene.h"



namespace Renderer
{
	class FileLoader
	{
	public:

		FileLoader();
		virtual ~FileLoader() {};

		virtual void loadFileData(std::string modelFilePath);
	};


	class ObjFileLoader : public FileLoader
	{
	public:
		ObjFileLoader() = delete;
		ObjFileLoader(Device* device);
		virtual ~ObjFileLoader();

		void loadFileData(std::string modelFilePath) override;
		std::vector<MeshData> GetMeshes();
		std::vector<Material> GetMaterials();
		Device* GetDevice();

	protected:
		Device* m_device;
		std::vector<MeshData> m_meshes;
		std::vector<Material> m_materials;
	};

	class GLTFFileLoader :public FileLoader
	{
	public:
		GLTFFileLoader();
		virtual ~GLTFFileLoader();

		void loadFileData(std::string modelFilePath) override;

		std::vector<GLTFMeshData> m_meshes;
	};
}