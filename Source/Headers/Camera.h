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

	struct InputStatus
	{
		glm::vec2 lastMousePos;
		glm::vec2 mouseDelta;
		bool isLeftMouseButtonDown;
		bool isRightMouseButtonDown;
		bool shouldRotate;

		bool shouldMove;
		bool moveForward;
		bool moveBackward;
		bool moveLeft;
		bool moveRight;
		bool moveUp;
		bool moveDown;
	};


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
		friend CameraController;
	public:
		Camera(Device* device, float aspectRatio);
		~Camera();

		VkBuffer GetBuffer() const;
		glm::mat4 GetViewmatrix();
		glm::mat4 GetProjectionMatrix();

		void RotateAroundUpAxis(float degree);
		void RotateAroundRightAxis(float degree);
		void RotateAroundForwardAxis(float degree);
		
		void UpdateViewMatrix(Handedness hand);
		void UpdateViewMatrixFromLookAt(Handedness hand);
		void UpdateBufferMemory();

		void DestroyVKResources();
		void Update();

		inline glm::vec3 Get3DVectorComponent(const glm::vec4 vec);
		inline glm::vec4 Set3DVectorComponent(const glm::vec3 vec);

		void HandleMouseInputEvent();
		void HandleKeyboardInputEvent();
		InputStatus m_cameraInputStatus;

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

		//This is all based on World Space
		glm::vec4 m_upVector_W;
		glm::vec4 m_forwardVector_W;
		glm::vec4 m_rightVector_W;

		float yaw=0.f;
		float pitch=0.f;

		glm::vec4 m_position_W;
		glm::vec4 m_lookTarget_W;


		void UpdateProjectionMatrix();
	};
}

