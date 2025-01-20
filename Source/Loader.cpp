#include "Loader.h"
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
			LoadModel(modelDataPath, modelFilePath, scene);

			glm::vec3 translation=glm::vec3(0,0,0);
			glm::vec3 rotation = glm::vec3(0, 0, 0);
			glm::vec3 scale=glm::vec3(0,0,0);

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
				SafeGetline(scene->fp_in, line);
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
				Primitive primitive;
				primitive.sphere.s = glm::vec4(position.x, position.y, position.z, radius);
				scene->AddPrimitive(primitive);
				SafeGetline(scene->fp_in, line);
			}
		}
	}

	void  Loader::LoadSceneCamera(Scene* scene)
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
			}
		}
	}

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