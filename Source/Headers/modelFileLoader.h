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
		virtual void loadDataToMesh(MeshData& m_mesh);
	};


	class ObjFileLoader : public ModelFileLoader
	{
	public:
		ObjFileLoader();
		~ObjFileLoader();

		void loadFileData(std::string modelFilePath) override;
		void loadDataToMesh(MeshData& m_mesh) override;

		std::vector<glm::vec3> vertex_pos;
		std::vector<glm::vec3> vertex_normal;
		std::vector<glm::vec2> vertex_texCoord;

		std::vector<glm::vec3> face_pos_idx;
		std::vector<glm::vec3> face_normal_idx;
		std::vector<glm::vec3> face_uv_idx;

		std::vector<std::string> texturePath;

		std::vector<int> pos_offset_idx;
		std::vector<int> normal_offset_idx;
		std::vector<int> texcoord_offset_idx;
		std::vector<int> face_offset_idx;

	};

	class PlyFiileLoader : public ModelFileLoader
	{

	};

	class GltfFileLoader :public ModelFileLoader
	{

	};
}