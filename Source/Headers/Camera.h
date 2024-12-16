#pragma once
#include <RendererInclude.h>
#include "SceneStructs.h"
#include "CameraController.h"
#include <Vulkan/Device.h>
#include <Vulkan/BufferUtils.h>

namespace Renderer
{
	struct CameraUniformBuffer
	{
		alignas(16) glm::mat4 viewMatrix;
		alignas(16) glm::mat4 projectionMatrix;
		alignas(16) glm::mat4 modelMatrix;
	};

	inline glm::vec3 Get3DVectorComponent(const glm::vec4 vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}
	inline glm::vec4 Set3DVectorComponent(const glm::vec3 vec)
	{
		return glm::vec4(vec.x, vec.y, vec.z, 0);
	}

	inline glm::vec4 Set3DPointComponent(const glm::vec3 vec)
	{
		return glm::vec4(vec.x, vec.y, vec.z, 1);
	}

	// Perform a linear interpolation
	inline double Lerp(float x0, float x1, float a)
	{
		return x0 + a * (x1 - x0);
	}

	//Perform a sphere interpolation
	enum class Handedness
	{
		RightHanded,
		LeftHanded,
	};

	class Camera
	{
	public:
		Camera(Device* device, float aspectRatio);
		~Camera();

		VkBuffer GetBuffer() const;
		glm::mat4 GetViewmatrix() const;
		glm::mat4 GetProjectionMatrix() const;
		float GetAspectRatio() const;
		float GetFOV() const;


		void UpdateViewMatrix(Handedness hand);
		void UpdateViewMatrixFromLookAt(Handedness hand, glm::vec3 upVec);
		void UpdateBufferMemory();

		void DestroyVKResources();

		void SetUpVector(glm::vec4 upVec);
		void SetLookTarget(glm::vec4 lookTarget);
		void SetRightVector(glm::vec4 rightVec);
		void SetForwardVector(glm::vec4 forward);
		void SetPosition(glm::vec4 position);

		glm::vec4 GetUpVector() const;
		glm::vec4 GetLookTarget() const;
		glm::vec4 GetRightVector() const;
		glm::vec4 GetForwardVector() const;
		glm::vec4 GetPosition() const;

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
		float m_movingSpeed=300.f;

		bool m_dirty;

		glm::mat4 m_projectionMatrix;
		glm::mat4 m_viewMatrix;

		//This is all based on World Space(used for 6Dof)
		glm::vec4 m_upVector_W;
		glm::vec4 m_forwardVector_W;
		glm::vec4 m_rightVector_W;

		glm::vec4 m_position_W;
		glm::vec4 m_lookTarget_W;


		void UpdateProjectionMatrix();
	};
}

