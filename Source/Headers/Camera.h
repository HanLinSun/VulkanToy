#pragma once
#include <RendererInclude.h>
#include "SceneStructs.h"
#include "Vulkan/Device.h"
#include "Headers/Vulkan/BufferUtils.h"

namespace Renderer
{
	struct CameraUniformBuffer
	{
		alignas(16) glm::mat4 viewMatrix;
		alignas(16) glm::mat4 projectionMatrix;
		alignas(16) glm::mat4 modelMatrix;
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
		
		void RotateAroundUpAxis(float degree);
		void RotateAroundRightAxis(float degree);
		void RotateAroundForwardAxis(float degree);
		
		void UpdateViewMatrix();

		void UpdateBufferMemory();

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
		float m_movingSpeed=400.f;

		glm::mat4 m_projectionMatrix;
		glm::mat4 m_viewMatrix;

		glm::vec4 m_upVector;
		glm::vec4 m_forwardVector;
		glm::vec4 m_rightVector;

		glm::vec3 m_rotation;
		glm::vec4 m_position;
		glm::vec4 m_lookAtPoint;

		float r, theta, phi;

		void UpdateProjectionMatrix();
	};
}

