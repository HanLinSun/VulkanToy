struct Intersection
{
	vec3 position;
	vec3 normal;
	float t;
};

struct Material {
	uint materialType;
	vec3 albedo;
	vec3 specular;
	vec3 roughness;
};

struct Triangle
{
	vec4 v1;
	vec4 v2;
	vec4 v3;
	uint materialIdx;
};

struct Sphere
{
	float radius;
	vec3 position;
	uint materialIdx;
};