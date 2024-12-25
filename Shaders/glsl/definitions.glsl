#ifndef SHARED_GLSL
#define SHARED_GLSL

#define PI 3.1415926535897932385
#define EPSILON 0.0003
#define INFINITY  1000000.0
#define MINIMUM   0.00001

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

struct Material 
{
	vec3 baseColor;
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
	float transmission;

	float opacity;
	int alphaMode;
	float alphaCutoff;

	int albedoTextureID; //base color map
	int normalTextureID;
	int metallicRoughnessTextureID;
	int emissonMapTextureID;

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
};

struct Light
{
	vec3 position;
	vec3 emission;
	vec3 u;
	vec3 v;
	float area;
	int type;
	float radius;
};


struct RayTraceUniformBuffer
{
	mat4 viewMatrix;
	mat4 projMatrix;
	vec3 cameraPos;
	uint lightNums;
	uint numTriangles;
	uint numSpheres;
	uint spp;
	uint maxDepth;
	float focalDistance;
	float cameraFOV;
};

#endif
