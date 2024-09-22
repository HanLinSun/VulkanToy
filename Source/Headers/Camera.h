#pragma once
#include <RendererInclude.h>
#include "SceneStructs.h"
#include "Vulkan/Device.h"
#include "Headers/Vulkan/BufferUtils.h"

namespace Renderer
{
	struct CameraUniformBuffer
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	class Camera
	{
	public:
		Camera(Device* device, float aspectRatio);
		~Camera();

		VkBuffer GetBuffer() const;
		void UpdateOrbit(float deltaX, float deltaY, float deltaZ);
		
	private:
		Device* m_device;
		CameraUniformBuffer m_cameraBufferObject;
		VkBuffer m_buffer;
		VkDeviceMemory m_bufferMemory;

		void* m_mappedData;
		float r, theta, phi;

	};
}

