#include <glm/glm.hpp>

enum LightType
{
	AreaLight=0,
	PointLight=1,
	DirectionalLight=2,
	InfiniteLight=3,
};

class LightCPU
{

private:
	glm::vec3 point;
	glm::vec3 surface_normal;
	glm::vec3 shading_normal;

};