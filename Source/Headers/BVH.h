#pragma once
#include <glm/glm.hpp>
#include <GPUModel.h>
#include <vector>

namespace BVHBuildTool
{
	Boundbox CreateBoundingBox(SphereGPU sphere);
	Boundbox CreateBoundingBox(Triangle triangle);
	Boundbox CreateBoundingBox(std::vector<BVHObject>& objects);

	inline Boundbox Union(Boundbox& b1, Boundbox& b2);
	inline Boundbox Union(Boundbox& b1, glm::vec3 point);
	inline Boundbox Intersect(Boundbox& b1, Boundbox& b2);
	inline bool Overlaps(Boundbox& b1, Boundbox& b2);



	Boundbox CreateSurroundAABB_Box(Boundbox& box_A, Boundbox& box_B);

	std::vector<BVHNodeGPU> BuildBVHGPUNode( std::vector<BVHObject>& srcObjects);
	void BuildBVHRecursive(int node, std::vector<BVHObject>& srcObjects, std::vector<BVHNodeCPU>& bvhTree_cpu);
	

}