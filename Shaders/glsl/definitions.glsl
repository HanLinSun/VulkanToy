#define PI 3.1415926535897932385
#define EPSILON 0.0003
#define INF 1000000.0

struct Intersection
{
	vec3 position;
	vec3 normal;
	bool backFaceFlag;
	float t;
};

struct Medium
{
	int type;
	float density;
	vec3 color;
	float anisotropy;
};

struct Material {
	vec3 baseColor;
	float opacity;
	int alphaMode;
	float alphaCutoff;
	vec3 emission;
	float anisotropic;
	float metallic;
	float roughness;
	float subsurface;
	float specularTint;
	float sheen;
	float sheenTint;
	float clearcoat;
	float clearcoatRoughness;
	float specTrans;
	float ior;
	float ax;
	float ay;

};

struct Triangle
{
	vec3 v0;
	vec3 v1;
	vec3 v2;
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

