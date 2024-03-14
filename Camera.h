#pragma once
#include <RendererInclude.h>

namespace vulkanRenderer
{
	class Camera
	{
	public:
		Camera(glm::vec3 init_position);
		Camera();
		~Camera();

		void updateCameraPosition(glm::vec3 in_position);
		void updateCameraRotation(glm::vec3 in_rotaton);

		glm::vec3 getCameraPosition();

		glm::mat4 getViewMatrix();
		glm::mat4 getProjectionMatrix();
		glm::mat4 getMVPMatrix();

	private:
		glm::vec3 m_position;
		glm::vec3 m_rotation;
		glm::vec3 m_lookAt;
		glm::vec3 m_upVector;

		glm::mat4 m_projectionMatrix;
		glm::mat4 m_viewMatrix;
		glm::mat4 m_modelMatrix;

		glm::mat4 m_invViewMatrix;
		glm::mat4 m_invProjMatrix;

		glm::mat4 m_mvpMatrix;

		float m_fieldOfView;
		float m_nearClipPlane;
		float m_farClipPlane;

		float m_aspectRatio;

		void updateMatrix(float width, float height);
	};
}
