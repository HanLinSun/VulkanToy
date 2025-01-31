#ifndef SHARED_GLSL
#define SHARED_GLSL

#define PI 3.1415926535897932385
#define TWO_PI 6.28318530717958648
#define INV_PI 0.3183098861837906
#define INV_2PI 0.15915494309189533577

#define RECT_LIGHT 0
#define SPHERE_LIGHT 1
#define DIRECTIONAL_LIGHT 2

#define EPSILON 0.0003
#define INFINITY  1000000.0
#define MINIMUM   0.00001

struct Intersection
{
	vec3 position;
	vec3 normal;
	int backFaceFlag;
	uint material_ID;
	float eta;
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
	float clearcoatGloss;

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

struct LBVHNode
{
	vec3 pMin;
	vec3 pMax;
	int splitAxis;
	int secondChildOffset;
	int primitiveIdx;  // 0 -> interior node
};

struct Triangle
{
	vec3 v0;
	vec3 v1;
	vec3 v2;

	vec3 n0;
	vec3 n1;
	vec3 n2;
};

struct Sphere
{
	vec4 s; //x,y,z is center, w is radius
};

struct Primitive
{
	Triangle triangle;
	Sphere sphere;
	mat4 transform;
	mat4 inverseTransform;
	mat4 inverseTranspose;
	uint materialIdx;
	uint type;
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
	float radius;
	float area;
	float type;
};

struct RayTraceUniformBuffer
{
	mat4 projMatrix;
	mat4 viewMatrix;

	uint lightNums;
	uint numPrimitives;
	uint spp;
	uint maxDepth;
	float focalDistance;
	float aperture;
};

struct LightSample
{
	vec3 normal;
	vec3 emission;
	vec3 direction;
	float distance;
	float pdf;
};

struct BSDFSample
{
	vec3 f; // BSDF value of material
	vec3 L; //output direction
	float pdf;
};
#endif