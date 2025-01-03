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

		virtual void loadFileData(Scene* scene, std::string modelFilePath, std::string modelFolderPath);
		virtual void loadFileData(Scene*, std::string sceneDescriptionPath);
	};


	class ObjFileLoader : public FileLoader
	{
	public:
		ObjFileLoader() = delete;
		ObjFileLoader(std::shared_ptr<Device> device);
		virtual ~ObjFileLoader();

		void loadFileData(Scene* scene, std::string modelFilePath, std::string modelFolderPath) override;
		virtual void loadFileData(Scene*, std::string sceneDescriptionPath);

		std::vector<MeshData> GetMeshes();
		std::shared_ptr<Device> GetDevice();

	protected:
		std::shared_ptr<Device> m_device;
		std::vector<MeshData> m_meshes;
	};

	class GLTFFileLoader :public FileLoader
	{
	public:
		GLTFFileLoader();
		virtual ~GLTFFileLoader();

		void loadFileData(Scene* scene, std::string modelFilePath,std::string modelFolderPath) override;
		virtual void loadFileData(Scene*, std::string sceneDescriptionPath);
		std::vector<GLTFMeshData> m_meshes;
	};
}