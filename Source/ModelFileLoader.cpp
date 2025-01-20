#include "modelFileLoader.h"
#include <iostream>
#include <set>
#include <Log.h>
#define TINYOBJLOADER_IMPLEMENTATION 
#include <tiny_obj_loader.h>
#include <glm/gtc/matrix_inverse.hpp>

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

	FileLoader::FileLoader() {}
	void FileLoader::loadFileData(Scene* scene,std::string path, std::string modelFolderPath) {}
	void FileLoader::loadFileData(Scene*, std::string sceneDescriptionPath){}



	ObjFileLoader::ObjFileLoader(std::shared_ptr<Device> device):m_device(device) {}

	ObjFileLoader:: ~ObjFileLoader()
	{
		//Clear will nt set capacity to 0, so need to use swap to free the real memory 
		m_meshes.clear();
		std::vector<MeshData>().swap(m_meshes);
	};


	std::vector<MeshData> ObjFileLoader::GetMeshes()
	{
		return m_meshes;
	}


    std::shared_ptr<Device> ObjFileLoader::GetDevice()
	{
		return m_device;
	}
	void ObjFileLoader::loadFileData(Scene* scene, std::string sceneDescriptionPath)
	{
		//Parse scene 
		
	}

	void ObjFileLoader::loadFileData(Scene* scene, std::string modelFilePath, std::string modelFolderPath)
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
			
		//Read material
		for (auto& material : materials)
		{
			std::shared_ptr<Material> load_mat = std::make_shared<Material>();
			load_mat->SetAmbientColor(glm::vec4(material.ambient[0], material.ambient[1], material.ambient[2], 1));
			load_mat->SetDiffuseColor(glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 1));
			load_mat->SetEmissiveColor(glm::vec4(material.emission[0], material.emission[1], material.emission[2], 1));
			load_mat->SetSpecularColor(glm::vec4(material.specular[0], material.specular[1], material.specular[2], 1));
			load_mat->SetTransmittance(glm::vec4(material.transmittance[0], material.transmittance[1], material.transmittance[2], 1));
			load_mat->SetOpacity(material.dissolve);
			load_mat->SetIndexOfRefraction((float)material.ior);
			load_mat->SetRoughness(material.roughness);
			load_mat->SetMetallic(material.metallic);
			load_mat->SetSheen(material.sheen);
	
			load_mat->SetClearCoatGloss(material.clearcoat_roughness);
			load_mat->SetClearCoat(material.clearcoat_thickness);

			load_mat->m_name = material.name;
			//This is ugly and may need to  use MACRO instead in the future

			if (material.ambient_texname.size()!=0)
			{ 
				std::shared_ptr<Texture2D> ambient_texture=std::make_shared<Texture2D>();
				if (ambient_texture->LoadFromFile(modelFolderPath + material.ambient_texname, VK_FORMAT_R8G8B8A8_SRGB, GetDevice()) == 0)
				{
					load_mat->SetTexture(scene->GetCurrentTextureSize(), TextureType::Ambient);
					scene->AddTexture(ambient_texture);
				}
			}

			if (material.normal_texname.size() != 0)
			{
				std::shared_ptr<Texture2D> normal_texture = std::make_shared<Texture2D>();
				if (normal_texture->LoadFromFile(modelFolderPath + material.normal_texname, VK_FORMAT_R8G8B8A8_SRGB, GetDevice()) == 0)
				{
					load_mat->SetTexture(scene->GetCurrentTextureSize(), TextureType::Normal);
					scene->AddTexture(normal_texture);
				}
			}

			if (material.alpha_texname.size() != 0)
			{
				std::shared_ptr<Texture2D> alpha_texture = std::make_shared<Texture2D>();
				if (alpha_texture->LoadFromFile(modelFolderPath + material.alpha_texname, VK_FORMAT_R8G8B8A8_SRGB, GetDevice()) == 0)
				{
					load_mat->SetTexture(scene->GetCurrentTextureSize(), TextureType::Opacity);
					scene->AddTexture(alpha_texture);
				}
			}

			if (material.bump_texname.size() != 0)
			{
				std::shared_ptr<Texture2D> bump_texture = std::make_shared<Texture2D>();
				if (bump_texture->LoadFromFile(modelFolderPath + material.bump_texname, VK_FORMAT_R8G8B8A8_SRGB, GetDevice()) == 0)
				{
					load_mat->SetTexture(scene->GetCurrentTextureSize(), TextureType::Bump);
					scene->AddTexture(bump_texture);
				}
			}

			if (material.reflection_texname.size() != 0)
			{
				std::shared_ptr<Texture2D> reflection_texture = std::make_shared<Texture2D>();
				if (reflection_texture->LoadFromFile(modelFolderPath + material.reflection_texname, VK_FORMAT_R8G8B8A8_SRGB, GetDevice()) == 0)
				{
					load_mat->SetTexture(scene->GetCurrentTextureSize(), TextureType::Reflection);
					scene->AddTexture(reflection_texture);
				}
			}
			scene->AddMaterial(load_mat);
		}

		glm::vec3 temp_position=glm::vec3(0,0,0);
        glm::vec3 temp_normal=glm::vec3(0,0,0);
		glm::vec3 temp_color=glm::vec3(1,1,1);
        glm::vec2 temp_uv=glm::vec2(0,0);

		glm::mat4 defaultTransformMat = glm::mat4(1.0f);
		glm::mat4 defaultInverseTransformMat = glm::inverse(defaultTransformMat);
		glm::mat4 defaultInverseTransposeMat = glm::inverseTranspose(defaultTransformMat);

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (size_t s = 0; s < shapes.size(); s++)
		{
			size_t index_offset = 0;
			MeshData mesh;
			uniqueVertices.clear();
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
				std::vector<Vertex> triVert;
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
					triVert.push_back(vert);

					if (uniqueVertices.count(vert) == 0) {
						uniqueVertices[vert] = static_cast<uint32_t>(mesh.m_vertices.size());
						mesh.m_vertices.push_back(vert);
					}
					mesh.m_indices.push_back(uniqueVertices[vert]);
				}
				index_offset += fv;
				// per-face material
				Primitive primitive;
				primitive.triangle.position_0 = triVert[0].position;
				primitive.triangle.position_1 = triVert[1].position;
				primitive.triangle.position_2 = triVert[2].position;

				primitive.triangle.normal_0 = triVert[0].normal;
				primitive.triangle.normal_1= triVert[1].normal;
				primitive.triangle.normal_2 = triVert[2].normal;
				//shapes[s].mesh.material_ids[f];
				primitive.material_ID = shapes[s].mesh.material_ids[f];
				primitive.transformMatrix = defaultTransformMat;
				primitive.inverseTransform = defaultInverseTransformMat;
				primitive.inverseTranspose = defaultInverseTransposeMat;
				mesh.m_primitives.push_back(primitive);

				//scene->AddTriangle(triangle);

				mesh.m_materialID = shapes[s].mesh.material_ids[f];
			}
			m_meshes.push_back(mesh);
		}


	}

	void GLTFFileLoader::loadFileData(Scene* scene, std::string sceneDescriptionPath)
	{
	}

}


