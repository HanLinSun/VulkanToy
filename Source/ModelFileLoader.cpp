#include "modelFileLoader.h"
#include <iostream>
#include <set>
#include <Log.h>
#define TINYOBJLOADER_IMPLEMENTATION 
#include <tiny_obj_loader.h>

namespace std {
	template<> struct hash<Renderer::Vertex> {
		size_t operator()(Renderer::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

namespace Renderer
{

	ModelFileLoader::ModelFileLoader() {}
	void ModelFileLoader::loadFileData(std::string path) {}
	ObjFileLoader::ObjFileLoader() {}

	ObjFileLoader:: ~ObjFileLoader()
	{
		//Clear will nt set capacity to 0, so need to use swap to free the real memory 
		m_meshes.clear();
		std::vector<MeshData>().swap(m_meshes);
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
		glm::vec3 temp_color=glm::vec3(1,1,1);
        glm::vec2 temp_uv;

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (size_t s = 0; s < shapes.size(); s++)
		{
			size_t index_offset = 0;
			MeshData mesh;
			uniqueVertices.clear();
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				for (size_t v = 0; v < fv; v++)
				{
					//access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
					temp_position = glm::vec3(vx, vy, vz);

					if (idx.normal_index >= 0) {
						tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
						tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
						tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
						temp_normal = glm::vec3(nx, ny, nz);
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0) {
						tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
						tinyobj::real_t ty =  1.0f -attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
						temp_uv = glm::vec2(tx, ty);
					}

					if (attrib.colors.size() != 0)
					{
						tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
						tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
						tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];
						temp_color = glm::vec3(red, green, blue);
					}

					Vertex vert = { temp_position,temp_normal,temp_color,temp_uv };
					if (uniqueVertices.count(vert) == 0) {
						uniqueVertices[vert] = static_cast<uint32_t>(mesh.m_vertices.size());
						mesh.m_vertices.push_back(vert);
					}
					mesh.m_indices.push_back(uniqueVertices[vert]);
				}
				index_offset += fv;
				// per-face material
				//shapes[s].mesh.material_ids[f];
			}
			m_meshes.push_back(mesh);
		}

	}

}


