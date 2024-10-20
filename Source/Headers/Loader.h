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
		void LoadModel(std::string path, std::string folder_path, ModelGroup* modelgroup);
	protected:
		std::shared_ptr<Device> m_device;
		VkCommandPool m_commandPool;
	};

}
