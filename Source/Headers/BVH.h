#pragma once
#include <glm/glm.hpp>
#include <GPUModel.h>
#include <vector>
#include <memory>
static int leafNodes = 0;
static int totalLeafNodes = 0;
static int totalPrimitives = 0;
static int interiorNodes = 0;

namespace BVHBuildTool
{
	Boundbox CreateBoundingBox(Sphere sphere);
	Boundbox CreateBoundingBox(Triangle triangle);
	Boundbox CreateBoundingBox(Primitive* prim);
	//Boundbox CreateBoundingBox(std::vector<BVHObject>& objects);

	Boundbox UnionBox(Boundbox& b1, Boundbox& b2);
	Boundbox UnionBox(Boundbox& b1, glm::vec3& point);
	Boundbox Intersect(Boundbox& b1, Boundbox& b2);
	bool Overlaps(Boundbox& b1, Boundbox& b2);


	Boundbox CreateSurroundAABB_Box(Boundbox& box_A, Boundbox& box_B);



	// Utility structure to keep track of the initial triangle and sphere index in the triangles array while sorting.
	struct BVHPrimitiveInfo
	{
		BVHPrimitiveInfo() {}
		BVHPrimitiveInfo(size_t primitiveNumber, const Boundbox& bounds)
			:primitiveNumber(primitiveNumber), centroid(0.5f * bounds.pMin + 0.5f * bounds.pMax) {
		}
		size_t primitiveNumber;
		Boundbox boundbox;
		glm::vec3 centroid;
	};

	struct BucketInfo {
		int count = 0;
		Boundbox bounds;
	};

	struct BVHBuildNode
	{
		Boundbox bounds;
		BVHBuildNode* children[2];
		int splitAxis, firstPrimOffset, nPrimitives;

		void InitLeaf(int first, int n, const Boundbox& b)
		{
			firstPrimOffset = first;
			nPrimitives = n;
			bounds = b;
			children[0] = children[1] = nullptr;
			++leafNodes;
			++totalLeafNodes;
			totalPrimitives += n;
		}
		void InitInterior(int axis, std::shared_ptr<BVHBuildNode> c0, std::shared_ptr<BVHBuildNode> c1) {
			children[0] = c0.get();
			children[1] = c1.get();
			Boundbox c0_box = c0->bounds;
			Boundbox c1_box = c1->bounds;
			bounds= UnionBox(c0_box, c1_box);
			splitAxis = axis;
			nPrimitives = 0;
			++interiorNodes;
		}

	};


	struct LinearBVHNode {
		Boundbox bounds;
		int primitivesOffset;   // leaf
		int secondChildOffset;  // interior
		uint16_t nPrimitives;  // 0 -> interior node
		uint8_t axis;          // interior node: xyz
		//uint8_t pad[1];        // ensure 32 byte total size
	};

	class BVHAccel
	{
	public:
		BVHAccel(std::vector<std::shared_ptr<Primitive>>& objects);
		~BVHAccel()=default;
		//Better way is to write a memory pool to handle memory allocate, which is more efficient
		//I use shared pointer instead so I do not need to release every treenode manually
		std::shared_ptr<BVHBuildNode> RecursiveBuild(std::vector<BVHPrimitiveInfo>& objects, int start, int end, int* totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrims);
		int FlattenBVH(BVHBuildNode* root, int* offset);

		std::vector<LinearBVHNodeGPU> GetLinearBVHGPUNode();
	private:
		std::vector<std::shared_ptr<Primitive>> primitives;
		std::vector<std::shared_ptr<LinearBVHNode>> LBVHNodes;
	};

}