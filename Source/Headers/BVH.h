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
		BVHPrimitiveInfo(int primitiveIdx, const Boundbox& bounds)
			:primitiveIndex(primitiveIdx), centroid(0.5f * bounds.pMin + 0.5f * bounds.pMax) {
		}
		int primitiveIndex=-1;
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
		BVHBuildNode* left;
		BVHBuildNode* right;
		int primitiveIdx=-1;
		int axis =0;
		void InitLeaf(int idx, const Boundbox& b)
		{
			primitiveIdx = idx;
			bounds = b;
			left = right = nullptr;
		}
		void InitInterior(int axis, BVHBuildNode* c0,BVHBuildNode* c1) {
			left = c0;
			right = c1;
			Boundbox c0_box = c0->bounds;
			Boundbox c1_box = c1->bounds;
			bounds= UnionBox(c0_box, c1_box);
		}
	};

	
	struct LinearBVHNode {

		LinearBVHNode()
		{
			primitiveIndex = -1;
		}

		Boundbox bounds;
		int axis;
		int secondChildIdx;
		int primitiveIndex;
	};

	class BVHAccel
	{
	public:
		BVHAccel(std::vector<std::shared_ptr<Primitive>>& objects);
		~BVHAccel()=default;
		//Better way is to write a memory pool to handle memory allocate, which is more efficient
		BVHBuildNode* RecursiveBuild(std::vector<BVHPrimitiveInfo>& objects,int& offset);
		int FlattenBVH(BVHBuildNode* root, int& offset);
		void ReleaseTreeMemory(BVHBuildNode* root);

		std::vector<LinearBVHNodeGPU> GetLinearBVHGPUNode();
	private:
		std::vector<std::shared_ptr<Primitive>> primitives;
		std::vector<LinearBVHNode> LBVHNodes;
	};

}