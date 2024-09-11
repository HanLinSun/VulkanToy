#include "modelFileLoader.h"
#include <iostream>
#include <set>
#define TINYOBJLOADER_IMPLEMENTATION 

namespace std {
	template<> struct hash<Renderer::Vertex> {
		size_t operator()(Renderer::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}s
	};
}

namespace Renderer
{

	ModelFileLoader::ModelFileLoader() {}

	void ModelFileLoader::loadFileData(std::string path) {}
	void ModelFileLoader::loadDataToMesh(MeshData& meshdata) {}

	ObjFileLoader::ObjFileLoader() {}

	ObjFileLoader:: ~ObjFileLoader()
	{
		//Clear will nt set capacity to 0, so need to use swap to free the real memory 
		vertex_pos.clear();
		std::vector<glm::vec3>().swap(vertex_pos);
		vertex_normal.clear();
		std::vector<glm::vec3>().swap(vertex_normal);
		vertex_texCoord.clear();
		std::vector<glm::vec2>().swap(vertex_texCoord);
		face_pos_idx.clear();
		std::vector<glm::vec3>().swap(face_pos_idx);
		face_normal_idx.clear();
		std::vector<glm::vec3>().swap(face_normal_idx);
		face_uv_idx.clear();
		std::vector<glm::vec3> swap(face_uv_idx);
	};


	void ObjFileLoader::loadFileData(std::string modelFilePath)
	{
		std::ifstream file;
		std::string s;
		
		tinyobj::ObjReader reader;
		tinyobj::ObjReaderConfig reader_config;

		if (!reader.ParseFromFile(modelFilePath, reader_config)) {
			if (!reader.Error().empty()) {
				std::cerr << "TinyObjReader: " << reader.Error();
			}
			exit(1);
		}

		if (!reader.Warning().empty()) {
			std::cout << "TinyObjReader: " << reader.Warning();
		}

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		glm::vec3 temp_position;
		glm::vec3 temp_normal;
		glm::vec2 temp_uv;

		//while (getline(file, s))
		//{
		//	if (s[0] == 'v' && s[1] == ' ')
		//	{
		//		//vertex
		//		std::vector<std::string> vert_pos = stringSplitHelper(s.substr(2, s.size() - 2), ' ');
		//		temp_position = glm::vec3(std::stof(vert_pos[0]), std::stof(vert_pos[1]), std::stof(vert_pos[2]));
		//		vertex_pos.push_back(temp_position);
		//	}
		//	else if (s[0] == 'v' && s[1] == 't')
		//	{
		//		//Texcoord 

		//		std::vector<std::string> uv_pos = stringSplitHelper(s.substr(3, s.size() - 2), ' ');
		//		temp_uv = glm::vec2(std::stof(uv_pos[0]), 1.f - std::stof(uv_pos[1]));
		//		vertex_texCoord.push_back(temp_uv);
		//	}
		//	else if (s[0] == 'v' && s[1] == 'n')
		//	{
		//		//normal
		//		std::vector<std::string> vert_normal = stringSplitHelper(s.substr(3, s.size() - 2), ' ');
		//		temp_position = glm::vec3(std::stof(vert_normal[0]), std::stof(vert_normal[1]), std::stof(vert_normal[2]));
		//		vertex_normal.push_back(temp_position);
		//	}
		//	else if (s[0] == 'f')
		//	{
		//		//Face index
		//		std::vector<std::string> face_idxStr = stringSplitHelper(s.substr(2, s.size() - 2), ' ');
		//		splitAndLoadFaceIndex(face_idxStr, face_pos_idx, face_uv_idx, face_normal_idx);
		//	}
		//}


	}

	void ObjFileLoader::loadDataToMesh(MeshData& m_mesh)
	{
		Triangle temp_triangle;
		Vertex vert_1, vert_2, vert_3;

		std::unordered_map<Vertex,uint32_t> vertexMap;
		
		for (int i = 0; i < face_pos_idx.size(); i++)
		{
			vert_1.position = vertex_pos[face_pos_idx[i].x-1];
			vert_1.normal = vertex_normal[face_normal_idx[i].x-1];
			vert_1.texCoord = vertex_texCoord[face_uv_idx[i].x-1];
			vert_1.color = glm::vec3(1, 1, 1);

			vert_2.position = vertex_pos[face_pos_idx[i].y-1];
			vert_2.normal = vertex_normal[face_normal_idx[i].y-1];
			vert_2.texCoord = vertex_texCoord[face_uv_idx[i].y-1];
			vert_2.color = glm::vec3(1, 1, 1);

			vert_3.position = vertex_pos[face_pos_idx[i].z-1];
			vert_3.normal = vertex_normal[face_normal_idx[i].z-1];
			vert_3.texCoord = vertex_texCoord[face_uv_idx[i].z-1];
			vert_3.color = glm::vec3(1, 1, 1);

			//vert_1 is new
			if (vertexMap.count(vert_1) ==0)
			{
				vertexMap[vert_1] = static_cast<uint32_t>(m_mesh.m_vertices.size());
			    m_mesh.m_vertices.push_back(vert_1);
			}


			if (vertexMap.count(vert_2) == 0)
			{
				vertexMap[vert_2] = static_cast<uint32_t>(m_mesh.m_vertices.size());
				m_mesh.m_vertices.push_back(vert_2);
			}

			if (vertexMap.count(vert_3) == 0)
			{
				vertexMap[vert_3] = static_cast<uint32_t>(m_mesh.m_vertices.size());
				m_mesh.m_vertices.push_back(vert_3);
			}
			
			m_mesh.m_index.push_back(vertexMap[vert_1]);
			m_mesh.m_index.push_back(vertexMap[vert_2]);
			m_mesh.m_index.push_back(vertexMap[vert_3]);
		}
	}
}


