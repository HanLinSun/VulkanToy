#ifndef SHARED_GLSL
#define SHARED_GLSL

#define PI 3.1415926535897932385
#define INV_PI 0.3183098861837906
#define INV_2PI 0.15915494309189533577;

#define PiOver2  1.57079632679489661923;
#define PiOver4  0.78539816339744830961;

#define EPSILON 0.0003
#define INFINITY  1000000.0
#define MINIMUM   0.00001

struct Intersection
{
	vec3 position;
	vec3 normal;
	int backFaceFlag;
	uint materialIndex;
	float t;
};

struct Medium
{
	int type;
	float density;
	vec3 color;
	float anisotropy;
};

struct PBRMaterial
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

struct NormalMaterial
{
	uint materialType;
	vec3 albedo;
	double refraction_index;
};

struct BVHNode
{
	vec3 min;
	vec3 max;
	int leftNodeIndex;
	int rightNodeIndex;
	int isLeaf;
	int axis;
	int triangleIndex;
	int sphereIndex;
};

struct Triangle
{
	vec3 v0;
	vec3 v1;
	vec3 v2;

	vec3 n0;
	vec3 n1;
	vec3 n2;

	mat4 transform;
	mat4 inverseTransform;
	mat4 inverseTranspose;
	uint materialIdx;
};

struct Sphere
{
	vec4 sphere; //x,y,z is center, w is radius
	mat4 transformMatrix;
	mat4 inverseTransformMatrix;
	mat4 inverseTranspose;
	uint materialIdx;
};

struct Ray
{
	vec3 origin; //world space
	vec3 direction; //world space
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
	vec3 cameraPos;
	vec3 cameraLookAt;
	vec3 cameraUp;
	float cameraYaw;
	float cameraPitch;

	uint lightNums;
	uint numTriangles;
	uint numSpheres;
	uint spp;
	uint maxDepth;
	float focalDistance;
	float cameraFOV;
};
#endif