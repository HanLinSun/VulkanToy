#pragma once
#include <RendererInclude.h>
#include "SceneStructs.h"

namespace Renderer
{
	class Camera
	{
	public:
		Camera(glm::vec3 init_position);
		Camera();
		~Camera();

		void setCameraPosition(glm::vec3 in_position);
		void setCameraRotation(glm::vec3 in_rotaton);

		glm::vec3 getCameraPosition();

		glm::mat4 getViewMatrix();
		glm::mat4 getProjectionMatrix();
		glm::mat4 getMVPMatrix();

		void translateCamera(glm::vec3 in_direction);
		void rotateCamera(glm::vec3 in_eulerAngle);

		void setCameraWH(float width, float height);
		void setPerspectiveMatrix(float fov,float znear, float zfar);
		void updateViewMatrix();
		void updateMove();

	private:
		glm::vec3 m_position;
		glm::vec3 m_rotation;
		glm::vec3 m_lookAt;

		glm::vec3 m_upVector;
		glm::vec3 m_rightVector;
		glm::vec3 m_forwardVector;

		glm::mat4 m_projectionMatrix;
		glm::mat4 m_viewMatrix;
		glm::mat4 m_modelMatrix;

		glm::mat4 m_invViewMatrix;
		glm::mat4 m_invProjMatrix;

		glm::mat4 m_mvpMatrix;

		glm::vec3 m_refPosition;

		float m_fieldOfView;
		float m_nearClipPlane;
		float m_farClipPlane;

		float rotationSpeed = 1.0f;
		float movementSpeed = 1.0f;

		float m_aspectRatio;

		float m_screenWidth;
		float m_screenHeight;

		bool m_updated = true;
		//By default is Vulkan, need to flip
		bool flipY = true;

	};
}

