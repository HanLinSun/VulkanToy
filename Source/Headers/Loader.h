#pragma once
#include <string>
#include "modelFileLoader.h"

namespace Renderer
{
	class Loader
	{
	public:
		Loader()=delete;
		Loader(std::shared_ptr<Device> device, VkCommandPool commandPool);
		~Loader();
		void LoadModel(std::string path, std::string folder_path, Scene* scene);
		void LoadModel(Scene* scene, std::string type);
		void LoadModel(std::string path, std::string folder_path, Scene* scene, int materialID);
		void LoadSceneCamera(Scene* scene);
		void LoadFromSceneFile(Scene* scene, std::string sceneFile);
		void LoadSceneLight(Scene* scene);
		void LoadSceneMaterial(Scene* scene, std::string matName);

		std::istream& SafeGetline(std::istream& is, std::string& t);
		std::vector<std::string> TokenizeString(std::string str);

	protected:
		std::shared_ptr<Device> m_device;
		VkCommandPool m_commandPool;
	};

}
