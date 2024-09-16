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
		ObjFileLoader();
		virtual ~ObjFileLoader();

		void loadFileData(std::string modelFilePath) override;

		std::vector<MeshData> m_meshes;
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