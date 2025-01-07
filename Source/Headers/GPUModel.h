#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <memory>
#include <vector>
#include <Tools.h>

static int globalSphereNum=0;

struct RayTraceUniformData
{
	// Compute shader uniform block object
	alignas(16) glm::mat4 cam_viewMatrix;
	alignas(16) glm::mat4 cam_projectionMatrix;
	alignas(16) glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 0.0f);
	alignas(16) glm::vec3 camLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
	alignas(16) glm::vec3 cam_Up = glm::vec3(0.0f, 1.0f, 0.0f);
	//For tracing scene
	alignas(4) int lightNums;
	alignas(4) int triangleNums;
	alignas(4) int sphereNums;
	alignas(4) int meshNums;
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

struct Mesh
{
	alignas(4) uint32_t meshType; //0 is mesh, 1 is sphere
	alignas(4) uint32_t sphereIdx;
	alignas(4) uint32_t startTriangleIdx;
	alignas(4) uint32_t triangleNums;
	alignas(16) glm::mat4 transformMatrix;
	alignas(16) glm::mat4 inverseTransform;
	alignas(16) glm::mat4 inverseTranspose;
	alignas(4) uint32_t material_ID;
};

struct Triangle
{
	alignas(16) glm::vec3 position_0;
	alignas(16) glm::vec3 position_1;
	alignas(16) glm::vec3 position_2;

	alignas(16) glm::vec3 normal_0;
	alignas(16) glm::vec3 normal_1;
	alignas(16) glm::vec3 normal_2;

	alignas(4)uint32_t mesh_ID;
};

struct SphereGPU
{
	alignas(16) glm::vec4 s; // x,y,z is position, w is radius
	alignas(4) uint32_t material_ID;
};

class SphereCPU
{

public:

	SphereCPU()
	{
		id = globalSphereNum;
		globalSphereNum++;
	}

	Mesh BuildMesh()
	{
		Mesh mesh;
		mesh.sphereIdx = id;
		mesh.startTriangleIdx = -1;
		mesh.triangleNums = -1;
		mesh.meshType = 1;
		mesh.material_ID = material_ID;
		mesh.transformMatrix = Tools::BuildTransformMatrix(translate, rotate, scale);
		mesh.inverseTransform = glm::inverse(mesh.transformMatrix);
		mesh.inverseTranspose = glm::inverseTranspose(mesh.transformMatrix);

		return mesh;
	}

	SphereGPU GetGPUSphere()
	{
		SphereGPU gpuSphere;
		gpuSphere.s = glm::vec4(position.x, position.y, position.z, radius);
		gpuSphere.material_ID = material_ID;
		return gpuSphere;
	}

	glm::vec3 position;
	float radius;
	glm::vec3 translate;
	glm::vec3 rotate;
	glm::vec3 scale;
	int id;
	int material_ID;

private:

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


struct Boundbox
{
	glm::vec3 min;
	glm::vec3 max;

	glm::vec3 Centroid()
	{
		return 0.5f * min + 0.5f * max;
	}
};

// Node in a non recursive BVH for use on GPU.
struct BVHNodeGPU
{
	alignas(16) glm::vec3 min;
	alignas(16) glm::vec3 max;

	alignas(4) int leftNodeIndex;
	alignas(4) int rightNodeIndex;

	alignas(4) int isLeaf;
	alignas(4) int axis;

	alignas(4) int triangleIndex; //triangle buffer index
	alignas(4) int sphereIndex; //sphere buffer index

};

// Utility structure to keep track of the initial triangle and sphere index in the triangles array while sorting.
struct BVHObject
{
	Boundbox boundbox;
	glm::vec3 centroid;
	uint32_t triangle_index=-1;
	uint32_t sphere_index = -1;

	void ComputeCentroid()
	{
		centroid = 0.5f * (boundbox.min + boundbox.max);
	}
};

struct BVHNodeCPU
{
	Boundbox boundingBox;

	// index refers to the index in the array of bvh nodes. Used for sorting a flattened Bvh.
	int index = -1;

	int leftNodeIndex=-2;
	int rightNodeIndex=-2;

	int splitAxis=0;

	int triangleIndex;
	int sphereIndex;

	std::vector<BVHObject> objects;

	

	BVHNodeGPU GetBVHGPUModel()
	{
		//bool leaf = leftNodeIndex == -1 && rightNodeIndex == -1;
		BVHNodeGPU node;
		node.min = boundingBox.min;
		node.max =boundingBox.max;

		node.leftNodeIndex = leftNodeIndex;
		node.rightNodeIndex = rightNodeIndex;

		if (leftNodeIndex == -1 && rightNodeIndex == -1)
		{
			node.isLeaf = 1;
			node.triangleIndex = objects[0].triangle_index;
			node.sphereIndex = objects[0].sphere_index;
		}
		else
		{
			node.isLeaf = 0;
			node.triangleIndex = -1;
			node.sphereIndex = -1;
		}

		node.axis = splitAxis;



		return node;
	}

};

