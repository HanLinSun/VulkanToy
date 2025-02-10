#include "Loader.h"
#include "Light.h"
#include <iostream>
#include <fstream>
namespace Renderer
{
	Loader::Loader(std::shared_ptr<Device> device, VkCommandPool commandPool):m_device(device),m_commandPool(commandPool){}
	Loader::~Loader() 
	{
	}

	void Loader::LoadModel(Scene* scene,  std::string type)
	{
		if (strcmp(type.c_str(), "Mesh") == 0)
		{
			std::string line;
			SafeGetline(scene->fp_in, line);
			std::string modelFilePath;
			std::string modelDataPath;
			if (!line.empty() && scene->fp_in.good())
			{
				std::vector<std::string> tokens = TokenizeString(line);
				if (strcmp(tokens[0].c_str(), "ModelFilePath") == 0)
				{
					modelFilePath = tokens[1];
				}
			}

			SafeGetline(scene->fp_in, line);
			if (!line.empty() && scene->fp_in.good())
			{
				std::vector<std::string> tokens = TokenizeString(line);
				if (strcmp(tokens[0].c_str(), "ModelDataPath") == 0)
				{
					modelDataPath = tokens[1];
				}
			}


			//LoadModel(modelDataPath, modelFilePath, scene);

			glm::vec3 translation=glm::vec3(0,0,0);
			glm::vec3 rotation = glm::vec3(0, 0, 0);
			glm::vec3 scale=glm::vec3(0,0,0);
			int materialId = -1;
			SafeGetline(scene->fp_in, line);
			while (!line.empty() && scene->fp_in.good()) {
				std::vector<std::string> tokens = TokenizeString(line);

				//load tranformations
				if (strcmp(tokens[0].c_str(), "Translate") == 0) {
					translation = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
				}
				else if (strcmp(tokens[0].c_str(), "Rotation") == 0) {
					rotation = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
				}
				else if (strcmp(tokens[0].c_str(), "Scale") == 0) {
					scale = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
				}
				else if (strcmp(tokens[0].c_str(), "MaterialID") == 0)
				{
					materialId = atoi(tokens[1].c_str());
				}
				SafeGetline(scene->fp_in, line);
			}

			if (materialId == -1)
			{
				LoadModel(modelDataPath, modelFilePath, scene);
			}
			else
			{
				LoadModel(modelDataPath, modelFilePath, scene, materialId);
			}


			scene->GetSceneModelGroupsRaw().back()->buildTransformationMatrix(translation, rotation, scale);
			scene->GetSceneModelGroupsRaw().back()->SetModelTransformMatrix();
			scene->GetPrimitivesFromModelGroups();
		}
		else if (strcmp(type.c_str(), "Sphere") == 0)
		{
			glm::vec3 position;
			float radius;
			std::string line;
			SafeGetline(scene->fp_in, line);
			while (!line.empty() && scene->fp_in.good())
			{
				std::vector<std::string> tokens = TokenizeString(line);
				if (strcmp(tokens[0].c_str(), "Position") == 0) {
					position = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
				}
				else if (strcmp(tokens[0].c_str(), "Radius") == 0)
				{
					radius = atof(tokens[1].c_str());
				}
				std::shared_ptr<Primitive> primitive =std::make_shared<Primitive>();
				primitive->sphere.s = glm::vec4(position.x, position.y, position.z, radius);
				scene->AddPrimitive(primitive);
				SafeGetline(scene->fp_in, line);
			}
		}
	}

	void Loader::LoadSceneLight(Scene* scene)
	{
		std::string line;
		SafeGetline(scene->fp_in, line);
		std::string type;
		glm::vec3 v1= glm::vec3(0, 0, 0);
		glm::vec3 v2 = glm::vec3(0, 0, 0);
		glm::vec3 position=glm::vec3(0,0,0);
		glm::vec3 emission=glm::vec3(0,0,0);
		float radius;
		while (!line.empty() && scene->fp_in.good())
		{
			std::vector<std::string> tokens = TokenizeString(line);
			if (strcmp(tokens[0].c_str(), "Type") == 0)
			{
				type = tokens[1];
			}
			else if (strcmp(tokens[0].c_str(), "Radius") == 0)
			{
				radius = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "Position") == 0)
			{
				position = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
			}
			else if (strcmp(tokens[0].c_str(), "v1") == 0)
			{
				v1 = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()),atof(tokens[3].c_str()));
			}
			else if (strcmp(tokens[0].c_str(), "v2") == 0)
			{
				v2 = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()),atof(tokens[3].c_str()));
			}
			else if (strcmp(tokens[0].c_str(), "Emission") == 0)
			{
				emission = glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()));
			}
			SafeGetline(scene->fp_in, line);
		}

		LightGPU light;
		light.position = position;
		light.emission = emission;
		if (type == "Quad")
		{
			light.type = LightType::QuadLight;
			light.u = v1 - light.position;
			light.v = v2 - light.position;
			light.area = glm::length(glm::cross(light.u, light.v));
		}
		else if (type == "Sphere")
		{
			light.type = LightType::SphereLight;
			light.radius = radius;
			light.area = 4.0f * 3.1415926535f * light.radius * light.radius;
		}
		else if (type == "Directional")
		{
			light.area = 0.0f;
			light.type = LightType::DirectionalLight;
		}
		scene->AddLight(light);
	}

	void Loader::LoadSceneMaterial(Scene* scene)
	{
		std::string line;
		SafeGetline(scene->fp_in, line);
		glm::vec4 baseColor = glm::vec4(0, 0, 0,1);
		glm::vec4 emission = glm::vec4(0, 0, 0, 1);
		float roughness = 0.0f;
		float metallic = 0.0f;
		float clearcoat = 0.0f;
		float clearcoatgloss = 0.0f;
		float sheen=0.0f;
		float sheenTint=0.0f;
		float spectrans = 0.0f;
		float subsurface = 0.0f;
		MaterialProperties materialProperty;

		while (!line.empty() && scene->fp_in.good())
		{
			std::vector<std::string> tokens = TokenizeString(line);
			if (strcmp(tokens[0].c_str(), "Color") == 0)
			{
				baseColor = glm::vec4(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()),0);
			}
			else if (strcmp(tokens[0].c_str(), "Roughness") == 0)
			{
				roughness = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "Metallic") == 0)
			{
				metallic = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "ClearCoat") == 0)
			{
				clearcoat = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "ClearCoatGloss") == 0)
			{
				clearcoatgloss = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "Sheen"))
			{
				sheen = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "SheenTint"))
			{
				sheenTint = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "SpecTrans") == 0)
			{
				spectrans = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "SubSurface") == 0)
			{
				subsurface = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "Emission") == 0)
			{
				emission = glm::vec4(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()), 0);
			}
			SafeGetline(scene->fp_in, line);
		}
		materialProperty.Diffuse = baseColor;
		materialProperty.Sheen = sheen;
		materialProperty.SheenTint = sheenTint;
		materialProperty.Emissive = emission;
		materialProperty.Roughness = roughness;
		materialProperty.Metallic = metallic;
		materialProperty.ClearCoat = clearcoat;
		materialProperty.ClearCoatGloss = clearcoatgloss;
		materialProperty.SpecTrans = spectrans;
		materialProperty.SubSurface = subsurface;
		std::shared_ptr<Material> material = std::make_shared<Material>(materialProperty);
		scene->AddMaterial(material);
	}


	void Loader::LoadSceneCamera(Scene* scene)
	{
		std::string line;
		SafeGetline(scene->fp_in, line);
		glm::vec2 resolution;
		float fov;
		glm::vec4 position;
		glm::vec4 lookAt;
		glm::vec4 upVec;

		while (!line.empty() && scene->fp_in.good())
		{
			std::vector<std::string> tokens = TokenizeString(line);
			if (strcmp(tokens[0].c_str(), "Resolution") == 0) {
				resolution = glm::vec2(atof(tokens[1].c_str()), atof(tokens[2].c_str()));
			}
			else if (strcmp(tokens[0].c_str(), "FOV") == 0)
			{
				fov = atof(tokens[1].c_str());
			}
			else if (strcmp(tokens[0].c_str(), "Position") == 0)
			{
				position = glm::vec4(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()),1.0f);
			}
			else if (strcmp(tokens[0].c_str(), "LookAt")==0)
			{
				lookAt = glm::vec4(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()),1.0f);
			}
			else if (strcmp(tokens[0].c_str(), "Up") == 0)
			{
				upVec = glm::vec4(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()), 1.0f);
			}
			SafeGetline(scene->fp_in, line);
		}

		auto cam = scene->GetCamera();
		glm::vec4 camForward =glm::normalize(lookAt - position);
		cam->SetForwardVector(camForward);
		cam->SetLookTarget(lookAt);
		cam->SetPosition(position);
		cam->SetUpVector(upVec);
		scene->SetSceneCamera(cam);
	}

	void Loader::LoadFromSceneFile(Scene* scene, std::string sceneFileName)
	{
		scene->fp_in.open(sceneFileName);
		if (!scene->fp_in.is_open()) {
			std::cerr << "Failed to open:"<< sceneFileName << std::endl;
		}

		while (scene->fp_in.good())
		{
			std::string line;
			SafeGetline(scene->fp_in, line);
			std::vector<std::string> tokens = TokenizeString(line);

			if (!line.empty())
			{
				//load tranformations
				if (strcmp(tokens[0].c_str(), "Object") == 0)
				{
					LoadModel(scene, tokens[1].c_str());
				}
				else if (strcmp(tokens[0].c_str(), "Camera") == 0)
				{
					LoadSceneCamera(scene);
				}
				else if (strcmp(tokens[0].c_str(), "Light") == 0)
				{
					LoadSceneLight(scene);
				}
				else if (strcmp(tokens[0].c_str(), "Material") == 0)
				{
					LoadSceneMaterial(scene);
				}
			}
		}
	}

	//This case, use mtl material
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
					Model* model=new Model(m_device.get(), m_commandPool, mesh.m_vertices, mesh.m_indices, mats[mesh.m_materialID],mesh.m_primitives);
					modelGroup->AddModel(model);
				}
				scene->AddModelGroup(std::move(modelGroup));
			}
		}
		
	}

	//use material settled by scene file
	void Loader::LoadModel(std::string path, std::string folder_path, Scene* scene, int materialID)
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
				objFileloader->loadFileData(scene, path, folder_path);
				std::vector<std::shared_ptr<Material>> mats = scene->GetMaterials();
				std::unique_ptr<ModelGroup> modelGroup = std::make_unique<ModelGroup>();
				for (auto& mesh : objFileloader->GetMeshes())
				{
					Model* model = new Model(m_device.get(), m_commandPool, mesh.m_vertices, mesh.m_indices, mats[materialID], mesh.m_primitives);
					modelGroup->AddModel(model);
				}
				scene->AddModelGroup(std::move(modelGroup));
			}
		}
	}

	std::istream& Loader::SafeGetline(std::istream& is, std::string& t) {
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

	std::vector<std::string> Loader::TokenizeString(std::string str) {
		std::stringstream strstr(str);
		std::istream_iterator<std::string> it(strstr);
		std::istream_iterator<std::string> end;
		std::vector<std::string> results(it, end);
		return results;
	}


}