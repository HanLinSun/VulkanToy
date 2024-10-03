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
		void LoadModel(std::string path, std::string folder_path, ModelGroup* modelgroup);
	protected:
		Device* m_device;
		VkCommandPool m_commandPool;
	};

}
