#pragma once
#include <glm/glm.hpp>
#include <GPUModel.h>
#include <vector>

namespace BVHBuildTool
{
	AABB_Box CreateAABB_Box(Sphere* sphere);
	AABB_Box CreateAABB_Box(Triangle* triangle);
	AABB_Box CreateAABB_Box(std::vector<BVHObject>& objects);

	AABB_Box CreateSurroundAABB_Box(AABB_Box& box_A, AABB_Box& box_B);

	std::vector<BVHNodeGPU> BuildBVH( std::vector<BVHObject>& srcObjects);

	bool NodeCompare(BVHNodeCPU& a, BVHNodeCPU& b);

	bool BoxCompare(AABB_Box& a, AABB_Box& b, int axis);
	bool BoxCompare_X(AABB_Box& a, AABB_Box& b);
	bool BoxCompare_Y(AABB_Box& a, AABB_Box& b);
	bool BoxCompare_Z(AABB_Box& a, AABB_Box& b);

}