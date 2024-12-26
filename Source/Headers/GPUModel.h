#pragma once
#include <glm/glm.hpp>

struct RayTraceUniformData
{
	// Compute shader uniform block object
	alignas(16) glm::mat4 cam_viewMatrix;
	alignas(16) glm::mat4 cam_projectionMatrix;
	alignas(16) glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 0.0f);
	//For tracing scene
	alignas(4) int lightNums;
	alignas(4) int triangleNums;
	alignas(4) int sphereNums;
	alignas(4) int samplePerPixel;
	alignas(4) int maxRecursiveDepth;
	alignas(4) glm::float32_t focalDistance;
	alignas(4) glm::float32_t cameraFOV = 10.0f;
	alignas(4) glm::f32 aspectRatio;
};


struct PBRMaterialData
{
	alignas(16) glm::vec4 baseColor;
	alignas(16) glm::vec4 emission;

	alignas(4) glm::float32_t anisotropic;
	alignas(4) glm::float32_t metallic;
	alignas(4) glm::float32_t roughness;
	alignas(4) glm::float32_t subsurface;

	alignas(4) glm::float32_t specularTint;
	alignas(4) glm::float32_t sheen;
	alignas(4) glm::float32_t sheenTint;

	alignas(4) glm::float32_t clearcoatThickness;
	alignas(4) glm::float32_t clearcoatRoughness;

	alignas(4) glm::float32_t specTrans;
	alignas(4) glm::float32_t ior;
	alignas(4) glm::float32_t transmission;

	alignas(4) glm::float32_t opacity;
	alignas(4) int alphaMode;
	alignas(4) glm::float32_t alphaCutoff;

	alignas(4) int albedoTextureID; //base color map
	alignas(4) int normalTextureID;
	alignas(4) int metallicRoughnessTextureID;
	alignas(4) int emissonMapTextureID;
};

struct Triangle
{
	alignas(16) glm::vec3 v0;
	alignas(16) glm::vec3 v1;
	alignas(16) glm::vec3 v2;
	alignas(4) uint32_t material_ID;
};


struct Sphere
{
	alignas(16) glm::vec4 s; // x,y,z is position, w is radius
	alignas(4) uint32_t materialIndex;
};


struct Light
{
	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 emission;
	alignas(16) glm::vec3 u;
	alignas(16) glm::vec3 v;
	alignas(4) float area;
	alignas(4) int type;
	alignas(4) float radius;
};
