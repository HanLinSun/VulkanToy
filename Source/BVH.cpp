#include <BVH.h>
#include <stack>
#include <algorithm>

const glm::vec3 eps(0.0001);

AABB_Box BVHBuildTool::CreateAABB_Box(Sphere* sphere)
{
	AABB_Box box;
	glm::vec3 radiusRange = glm::vec3(sphere->s.w, sphere->s.w, sphere->s.w);
	glm::vec3 spherePos = glm::vec3(sphere->s.x, sphere->s.y, sphere->s.z);
	box.min = spherePos - radiusRange;
	box.max = spherePos + radiusRange;
	return box;
}

AABB_Box BVHBuildTool::CreateAABB_Box(Triangle* triangle)
{
	return { glm::min(glm::min(triangle->v0, triangle->v1), triangle->v2) - eps, glm::max(glm::max(triangle->v0, triangle->v1), triangle->v2) + eps };
}

AABB_Box BVHBuildTool::CreateSurroundAABB_Box(AABB_Box& box_A, AABB_Box& box_B)
{
	return { glm::min(box_A.min, box_B.min), glm::max(box_A.max, box_A.max) };
}

AABB_Box BVHBuildTool::CreateAABB_Box(std::vector<BVHObject>& objects)
{
	AABB_Box tempBox;
	AABB_Box outputBox;
	bool firstBox = true;

	for (auto& object : objects)
	{
		if (object.triangle != nullptr)
		{
			tempBox = CreateAABB_Box(object.triangle);
		}
		else if (object.sphere != nullptr)
		{
			tempBox = CreateAABB_Box(object.sphere);
		}
		outputBox = firstBox ? tempBox : CreateSurroundAABB_Box(outputBox, tempBox);
		firstBox = false;
	}

	return outputBox;
}

bool BVHBuildTool::NodeCompare(BVHNodeCPU& a, BVHNodeCPU& b)
{
	return a.index < b.index;
}

bool BVHBuildTool::BoxCompare(AABB_Box& box_A, AABB_Box& box_B, int axis)
{
	return box_A.min[axis] < box_B.min[axis];
}

bool BVHBuildTool::BoxCompare_X(AABB_Box& a, AABB_Box& b)
{
	return BoxCompare(a, b, 0);
}

bool BVHBuildTool::BoxCompare_Y(AABB_Box& a, AABB_Box& b)
{
	return BoxCompare(a, b, 1);
}

bool BVHBuildTool::BoxCompare_Z(AABB_Box& a, AABB_Box& b)
{
	return BoxCompare(a, b, 2);
}

// Since GPU can't deal with tree structures we need to create a flattened BVH.
 // Stack is used instead of a tree.
std::vector<BVHNodeGPU> BVHBuildTool::BuildBVH(std::vector<BVHObject>& objects)
{
	std::vector<BVHNodeCPU> intermediate;
	int nodeCounter = 0;
	std::stack<BVHNodeCPU> nodeStack;

	BVHNodeCPU root;
	root.index = nodeCounter;
	nodeCounter++;
	nodeStack.push(root);

	while (!nodeStack.empty())
	{
		BVHNodeCPU currentNode = nodeStack.top();
		nodeStack.pop();
		currentNode.boundingBox = CreateAABB_Box(objects);
		int axis = currentNode.boundingBox.RandomAxis();
		auto comparator = (axis == 0) ? BoxCompare_X
			: (axis == 1) ? BoxCompare_Y
			: BoxCompare_Z;

		size_t objectSpan = objects.size();
		std::sort(currentNode.objects.begin(), currentNode.objects.end(), comparator);
		if (objectSpan <= 1)
		{
			intermediate.push_back(currentNode);
			continue;
		}
		else
		{
			auto mid = objectSpan / 2;
			BVHNodeCPU leftNode;
			leftNode.index = nodeCounter;
			for (int i = 0; i < mid; i++)
			{
				leftNode.objects.push_back(objects[i]);
			}
			nodeCounter++;
			nodeStack.push(leftNode);

			BVHNodeCPU rightNode;
			rightNode.index = nodeCounter;
			for(int i=mid;i<objectSpan;i++)
			{
				rightNode.objects.push_back(objects[i]);
			}
			nodeCounter++;
			nodeStack.push(rightNode);

			currentNode.leftNodeIndex = leftNode.index;
			currentNode.rightNodeIndex = rightNode.index;
			intermediate.push_back(currentNode);
		}
	}

	std::sort(intermediate.begin(), intermediate.end(), NodeCompare);

	std::vector<BVHNodeGPU> output;
	output.reserve(intermediate.size());
	for (int i = 0; i < intermediate.size(); i++)
	{
		output.push_back(intermediate[i].GetBVHGPUModel());
	}
	return output;

}


