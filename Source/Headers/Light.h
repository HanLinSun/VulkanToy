#pragma once
#include <RendererInclude.h>
namespace Renderer
{
	class Light
	{
	public:
		Light();
		virtual ~Light();

	    glm::vec3 getLightColor();
		void setLightColor(glm::vec3 in_color);

		glm::vec3 getLightPosition();
		void setLightPosition();

	private:
		glm::vec3 color;
		glm::vec3 light_direction;
		glm::vec3 light_position;
	};

	class PointLight : public Light
	{

	};

	class SpotLight : public Light
	{

	};

	class AreaLight : public Light
	{

	};
}
