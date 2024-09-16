#pragma once
#include <string>
#include "modelFileLoader.h"

namespace Renderer
{
	class Loader
	{
	public:
		Loader()=delete;
		Loader(Device* device, VkCommandPool commandPool);
		~Loader();
		void LoadModel(std::string path, std::string texture_path, std::vector<Model*>& models);
		void LoadModel(std::string path, std::vector<Model*>& models);
	protected:
		Device* m_device;
		VkCommandPool m_commandPool;
	};

}
