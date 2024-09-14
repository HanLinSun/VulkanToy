#pragma once
#include <vector>
#include <fstream>
#include <string>
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include "Scene.h"



namespace Renderer
{
	class ModelFileLoader
	{
	public:

		ModelFileLoader();
		virtual ~ModelFileLoader() {};

		virtual void loadFileData(std::string modelFilePath);
	};


	class ObjFileLoader : public ModelFileLoader
	{
	public:
		ObjFileLoader();
		~ObjFileLoader();

		void loadFileData(std::string modelFilePath) override;

		std::vector<MeshData> m_meshes;
	};

	class PlyFiileLoader : public ModelFileLoader
	{

	};

	class GltfFileLoader :public ModelFileLoader
	{

	};
}