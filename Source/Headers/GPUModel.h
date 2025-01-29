#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>



struct RayTraceUniformData
{
	// Compute shader uniform block object
	alignas(16) glm::mat4 camProjectionMatrix;
	alignas(16) glm::mat4 camViewMatrix;

	//For tracing scene
	alignas(4) int lightNums;
	alignas(4) int primNums;
	alignas(4) int samplePerPixel;
	alignas(4) int maxRecursiveDepth;
	alignas(4) glm::float32_t focalDistance;
	alignas(4) glm::float32_t aperture;
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

	alignas(4) glm::float32_t clearcoat;
	alignas(4) glm::float32_t clearcoatGloss;

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
	alignas(16) glm::vec3 position_0;
	alignas(16) glm::vec3 position_1;
	alignas(16) glm::vec3 position_2;

	alignas(16) glm::vec3 normal_0;
	alignas(16) glm::vec3 normal_1;
	alignas(16) glm::vec3 normal_2;
};


struct Sphere
{
	alignas(16) glm::vec4 s; // x,y,z is position, w is radius
};

struct LightGPU
{
	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 normal;
	alignas(16) glm::vec3 emission;
	alignas(4) int type;
};

struct Primitive
{
	Triangle triangle;
	Sphere sphere;
	alignas(16) glm::mat4 transformMatrix;
	alignas(16) glm::mat4 inverseTransform;
	alignas(16) glm::mat4 inverseTranspose;
	alignas(4) uint32_t material_ID;
	alignas(4) int type; //0 is triangle, 1 is sphere
};



class Boundbox
{
public:

	Boundbox() {
		float minNum = std::numeric_limits<float>::lowest();
		float maxNum = std::numeric_limits<float>::max();
		pMin = glm::vec3(maxNum, maxNum, maxNum);
		pMax = glm::vec3(minNum, minNum, minNum);
	}

	Boundbox(const glm::vec3& p1, const glm::vec3& p2)
		: pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
			std::min(p1.z, p2.z)),
		pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
			std::max(p1.z, p2.z)) {
	}

	// Copy constructor
	Boundbox(const Boundbox& other) = default;

	// Move constructor
	Boundbox(Boundbox&& other) noexcept = default;

	// Copy assignment
	Boundbox& operator=(Boundbox&& other) noexcept {
		if (this != &other) {
			this->pMin = std::move(other.pMin);
			this->pMax = std::move(other.pMax);
		}
		return *this;
	}

	glm::vec3 Diagonal() const
	{
		return pMax - pMin;
	}

	glm::vec3 Offset(const glm::vec3& point) const
	{
		glm::vec3 o = point - pMin;
		if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
		if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
		if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
		return o;
	}


	float SurfaceArea() const
	{
		glm::vec3 d = Diagonal();
		return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	//X,y,z get the longest
	int MaxExtent()
	{
		glm::vec3 d = Diagonal();
		if (d.x > d.y && d.x > d.z) return 0;
		else if (d.y > d.z  && d.y>d.x) return 1;
		else if(d.z>d.x && d.z>d.y)return 2;
	}

	// Copy assignment operator
	Boundbox& operator=(const Boundbox& other) {
		if (this != &other) {
			this->pMin = other.pMin;
			this->pMax = other.pMax;
		}
		return *this;
	}

	bool operator==(const Boundbox& b) const {
		return b.pMin == pMin && b.pMax == pMax;
	}
	bool operator!=(const Boundbox& b) const {
		return b.pMin != pMin || b.pMax != pMax;
	}

	glm::vec3 pMin;
	glm::vec3 pMax;
};

struct LinearBVHNode {
	Boundbox bounds;
	int primitivesOffset;   // leaf
	int secondChildOffset;  // interior
	uint16_t nPrimitives;  // 0 -> interior node
	uint8_t axis;          // interior node: xyz
	uint8_t pad[1];        // ensure 32 byte total size
};

struct  LinearBVHNodeGPU
{
	alignas(16) glm::vec3 pMin;
	alignas(16) glm::vec3 pMax;
	alignas(4) int leftNodeIdx;
	alignas(4) int rightNodeIdx;
	alignas(4) int primitiveIdx;  // 0 -> interior node
};




