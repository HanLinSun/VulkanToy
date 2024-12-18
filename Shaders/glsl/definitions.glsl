struct Intersection
{
	vec3 position;
	vec3 normal;
	bool backFaceFlag;
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
	vec3 v1;
	vec3 v2;
	vec3 v3;
	uint materialIdx;
};

struct Sphere
{
	vec4 sphere; //x,y,z is center, w is radius
	uint materialIdx;
};

struct Ray
{
	vec3 origin;
	vec3 direction;
	//float tMax;
	//Medium media;
};

