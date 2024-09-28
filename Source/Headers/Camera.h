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

	// Perform a linear interpolation
	inline double Lerp(float x0, float x1, float a)
	{
		return x0 + a * (x1 - x0);
	}

	class Camera
	{
	public:
		Camera(Device* device, float aspectRatio);
		~Camera();

		VkBuffer GetBuffer() const;
		void UpdateOrbit(float deltaX, float deltaY, float deltaZ);

		glm::mat4 GetViewmatrix();
		glm::mat4 GetProjectionMatrix();

		void UpdateTransform_X(float deltaX);
		void UpdateTransform_Y(float deltaY);
		void UpdateTransform_Z(float deltaZ);
		
		
		
	private:
		Device* m_device;
		CameraUniformBuffer m_cameraBufferObject;
		VkBuffer m_buffer;
		VkDeviceMemory m_bufferMemory;


		void* m_mappedData;
		float m_aspectRatio;
		float m_nearClipPlane;
		float m_farClipPlane;
		float m_fov;
		float m_movingSpeed=20.f;

		glm::mat4 m_projectionMatrix;
		glm::mat4 m_viewMatrix;

		glm::vec3 m_upVector;
		glm::vec3 m_forwardVector;
		glm::vec3 m_rightVector;

		glm::vec3 m_rotation;
		glm::vec3 m_position;
		glm::vec3 m_lookAtPoint;

		float r, theta, phi;

		void UpdateViewMatrix();
		void UpdateProjectionMatrix();
	};
}

