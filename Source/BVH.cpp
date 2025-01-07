#include <BVH.h>
#include <stack>
#include <algorithm>
#include <log.h>

const glm::vec3 eps(0.0001);

Boundbox BVHBuildTool::CreateBoundingBox(SphereGPU sphere)
{
	Boundbox box;
	glm::vec3 radiusRange = glm::vec3(sphere.s.w, sphere.s.w, sphere.s.w);
	glm::vec3 spherePos = glm::vec3(sphere.s.x, sphere.s.y, sphere.s.z);
	box.min = spherePos - radiusRange;
	box.max = spherePos + radiusRange;
	return box;
}

Boundbox BVHBuildTool::CreateBoundingBox(Triangle triangle)
{
	glm::vec3 minCorner = glm::min(triangle.position_0, glm::min(triangle.position_1, triangle.position_2));
	glm::vec3 maxCorner = glm::max(triangle.position_0, glm::max(triangle.position_1, triangle.position_2));
	return { minCorner,maxCorner };
}

Boundbox BVHBuildTool::CreateSurroundAABB_Box(Boundbox& box_A, Boundbox& box_B)
{
	return { glm::min(box_A.min, box_B.min), glm::max(box_A.max, box_A.max) };
}

Boundbox BVHBuildTool::CreateBoundingBox(std::vector<BVHObject>& objects)
{
	Boundbox outputBox;
	bool firstBox = true;

	for (auto& object : objects)
	{
		Boundbox tempBox = object.boundbox;
		outputBox = firstBox ? tempBox : CreateSurroundAABB_Box(outputBox, tempBox);
		firstBox = false;
	}
	return outputBox;
}


inline Boundbox BVHBuildTool::Union(Boundbox& b1, Boundbox& b2)
{
	Boundbox ret;
	ret.min = glm::min(b1.min, b2.min);
	ret.max = glm::max(b1.max, b2.max);
	return ret;
}

inline Boundbox BVHBuildTool::Union(Boundbox& b1, glm::vec3 point)
{
	Boundbox ret;
	ret.min = glm::min(b1.min,point);
	ret.max = glm::max(b1.max, point);
	return ret;
}

inline Boundbox BVHBuildTool::Intersect(Boundbox& b1, Boundbox& b2)
{
	Boundbox ret;
	ret.min = glm::max(b1.min, b2.min);
	ret.max = glm::min(b1.max, b2.max);
	return ret;
}

inline bool BVHBuildTool::Overlaps(Boundbox& b1, Boundbox& b2)
{
	bool x = (b1.max.x >= b2.min.x) && (b1.min.x <= b2.max.x);
	bool y = (b1.max.y >= b2.min.y) && (b1.min.y <= b2.max.y);
	bool z = (b1.max.z >= b2.min.z) && (b1.min.z <= b2.max.z);
	return (x && y && z);
}


//Based on https://pbr-book.org/4ed/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies
//Need to pass to compute shader so use array as binary tree
void BVHBuildTool::BuildBVHRecursive(int nodeIdx, std::vector<BVHObject>& srcObjects, std::vector<BVHNodeCPU>& bvhTree_cpu)
{
	if (nodeIdx >= bvhTree_cpu.size() * 2 - 1) return;
	bvhTree_cpu[nodeIdx].index = nodeIdx;

	bvhTree_cpu[nodeIdx].boundingBox = CreateBoundingBox(srcObjects);
	bvhTree_cpu[nodeIdx].leftNodeIndex = 2 * nodeIdx + 1;
	bvhTree_cpu[nodeIdx].rightNodeIndex = 2 * nodeIdx + 2;


	if (srcObjects.size() == 1)
	{
		bvhTree_cpu[nodeIdx].objects = srcObjects;
		bvhTree_cpu[nodeIdx].leftNodeIndex = -1;
		bvhTree_cpu[nodeIdx].rightNodeIndex = -1;
		bvhTree_cpu[nodeIdx].sphereIndex = srcObjects[0].sphere_index;
		bvhTree_cpu[nodeIdx].triangleIndex = srcObjects[0].triangle_index;
		return;
	}

	// estimate split axis by calculating maximum extent
	int& axis = bvhTree_cpu[nodeIdx].splitAxis;
	axis = 0; //Default use x
	glm::vec3 diagonal = bvhTree_cpu[nodeIdx].boundingBox.max - bvhTree_cpu[nodeIdx].boundingBox.min;
	if (diagonal.x > diagonal.y && diagonal.x > diagonal.z) axis = 0;
	else if (diagonal.y > diagonal.z) axis = 1;
	else axis = 2;

	//In the end need to build bvh in parallel if have too much nodes
	
	//Now just use sequential
    // Partition primitives into equally sized subsets
	int midIdx = srcObjects.size() / 2;
	std::nth_element(&srcObjects[0], &(srcObjects[midIdx]), &(srcObjects[srcObjects.size() - 1]) + 1,
		[axis](const BVHObject& a, const BVHObject& b)
		{
			return a.centroid[axis] < b.centroid[axis];
		});

	std::vector<BVHObject> leftObjects(srcObjects.begin(), srcObjects.begin() + midIdx);
	BuildBVHRecursive(bvhTree_cpu[nodeIdx].leftNodeIndex, leftObjects,bvhTree_cpu);

	std::vector<BVHObject> rightObjects(srcObjects.begin() + midIdx, srcObjects.end());
	BuildBVHRecursive(bvhTree_cpu[nodeIdx].rightNodeIndex, rightObjects,bvhTree_cpu);
}


std::vector<BVHNodeGPU> BVHBuildTool::BuildBVHGPUNode(std::vector<BVHObject>& objects)
{
	std::vector<BVHNodeCPU> bvhTree_CPU; //stored in bi-tree
	std::vector<BVHNodeGPU> output;
	if (objects.size() == 0)
	{
		LOG_CLIENT_ERROR("Build BVH Error: does not have objects to build, return empty vector");
		return output;
	}

	int depth = glm::ceil(glm::log2((float)objects.size()))+1;
	bvhTree_CPU.resize(glm::pow(2, depth) - 1); // full binary tree size
	//Use Equal split method to build
	//DFS
	//start from 0
	BuildBVHRecursive(0, objects, bvhTree_CPU);

	output.reserve(bvhTree_CPU.size());
	for (int i = 0; i < bvhTree_CPU.size(); i++)
	{
		output.push_back(bvhTree_CPU[i].GetBVHGPUModel());
	}
	return output;

}


