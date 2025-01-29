#include <BVH.h>
#include <queue>
#include <algorithm>
#include <log.h>

using namespace BVHBuildTool;

const glm::vec3 eps(0.0001);

static int totalNodes = 0;

Boundbox BVHBuildTool::CreateBoundingBox(Sphere sphere)
{
	Boundbox box;
	glm::vec3 radiusRange = glm::vec3(sphere.s.w, sphere.s.w, sphere.s.w);
	glm::vec3 spherePos = glm::vec3(sphere.s.x, sphere.s.y, sphere.s.z);
	box.pMin = spherePos - radiusRange;
	box.pMax = spherePos + radiusRange;
	return box;
}

Boundbox BVHBuildTool::CreateBoundingBox(Triangle triangle)
{
	glm::vec3 minCorner = glm::min(triangle.position_0, glm::min(triangle.position_1, triangle.position_2));
	glm::vec3 maxCorner = glm::max(triangle.position_0, glm::max(triangle.position_1, triangle.position_2));
	Boundbox box;
	box.pMax = maxCorner;
	box.pMin = minCorner;
	return box;
}

Boundbox BVHBuildTool::CreateBoundingBox(Primitive* prim)
{
	if (prim->type == 0)
	{
		return CreateBoundingBox(prim->triangle);
	}
	else if (prim->type == 1)
	{
		return CreateBoundingBox(prim->sphere);
	}
}

Boundbox BVHBuildTool::CreateSurroundAABB_Box(Boundbox& box_A, Boundbox& box_B)
{
	return { glm::min(box_A.pMin, box_B.pMin), glm::max(box_A.pMax, box_A.pMax) };
}



Boundbox BVHBuildTool::UnionBox(Boundbox& b1, Boundbox& b2)
{
	Boundbox ret;
	ret.pMin = glm::min(b1.pMin, b2.pMin);
	ret.pMax = glm::max(b1.pMax, b2.pMax);
	return ret;
}


Boundbox BVHBuildTool::UnionBox(Boundbox& b1, glm::vec3& point)
{
	Boundbox ret;
	ret.pMin = glm::min(b1.pMin,point);
	ret.pMax = glm::max(b1.pMax, point);
	return ret;
}

Boundbox BVHBuildTool::Intersect(Boundbox& b1, Boundbox& b2)
{
	Boundbox ret;
	ret.pMin = glm::max(b1.pMin, b2.pMin);
	ret.pMax = glm::min(b1.pMax, b2.pMax);
	return ret;
}

bool BVHBuildTool::Overlaps(Boundbox& b1, Boundbox& b2)
{
	bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
	bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
	bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
	return (x && y && z);
}

BVHBuildNode* BVHAccel::RecursiveBuild(std::vector<BVHPrimitiveInfo>& primitiveInfo)
{
	BVHBuildNode* node = new BVHBuildNode();
	totalNodes++;
	// Compute bounds of all primitives in BVH node
	Boundbox bounds;
	for (int i = 0; i < primitiveInfo.size(); ++i)
		bounds = BVHBuildTool::UnionBox(bounds, primitiveInfo[i].boundbox);

	int objectSize = primitiveInfo.size();
	if (objectSize == 1)
	{
		// Create leaf _BVHBuildNode_
		node->bounds = primitiveInfo[0].boundbox;
		node->primitiveIdx =primitiveInfo[0].primitiveIndex;
		node->left = nullptr;
		node->right = nullptr;
		return node;
	}
	else if(objectSize==2)
	{
		std::vector<BVHPrimitiveInfo> primLeft;
		std::vector<BVHPrimitiveInfo> primRight;
		primLeft.push_back(primitiveInfo[0]);
		primRight.push_back(primitiveInfo[1]);
		node->left = RecursiveBuild(primLeft);
		node->right = RecursiveBuild(primRight);
		node->bounds = UnionBox(node->left->bounds, node->right->bounds);

		return node;
	}
	else
	{
		Boundbox centroidBound;
		for (int i = 0; i < primitiveInfo.size(); i++)
		{
			centroidBound = UnionBox(centroidBound, primitiveInfo[i].centroid);
		}

		switch (centroidBound.MaxExtent())
		{
		case 0:
			// X
			std::sort(primitiveInfo.begin(), primitiveInfo.end(), [](auto f1, auto f2)
				{
					return f1.centroid.x < f2.centroid.x;
				});
			break;
		case 1:
			// Y
			std::sort(primitiveInfo.begin(), primitiveInfo.end(), [](auto f1, auto f2)
				{
					return f1.centroid.y < f2.centroid.y;
				});
			break;
		case 2:
			// Z
			std::sort(primitiveInfo.begin(), primitiveInfo.end(), [](auto f1, auto f2)
				{
					return f1.centroid.z < f2.centroid.z;
				});
			break;
		}

		const auto& begin = primitiveInfo.begin();
		const auto& end = primitiveInfo.end();

		constexpr uint8_t SlashCount = 12;
		constexpr float SlashCountInv = 1.0f / static_cast<float>(SlashCount);
		const float SC = centroidBound.SurfaceArea();

		uint8_t minCostIndex = SlashCount / 2;
		float minCost = std::numeric_limits<float>::infinity();

		for (uint8_t index = 1; index < SlashCount; ++index)
		{
			const auto& target = primitiveInfo.begin() + (primitiveInfo.size() * index * SlashCountInv);
			auto leftObjects = std::vector<BVHPrimitiveInfo>(begin, target);
			auto rightObjects = std::vector<BVHPrimitiveInfo>(target, end);

			// Surface area of the boundbox of two divided parts
			Boundbox leftBounds, rightBounds;
			for ( auto& obj : leftObjects)
			{
				leftBounds = UnionBox(leftBounds, obj.centroid);
			}

			for ( auto& obj : rightObjects)
			{
				rightBounds = UnionBox(rightBounds, obj.centroid);
			}

			float SA = leftBounds.SurfaceArea();
			float SB = rightBounds.SurfaceArea();
			float a = leftObjects.size();
			float b = rightObjects.size();
			float cost = (SA * a + SB * b) / SC + 0.125f;

			if (cost < minCost)
			{
				minCost = cost;
				minCostIndex = index;
			}
		}
		const auto& target = primitiveInfo.begin() + (primitiveInfo.size() * minCostIndex * SlashCountInv);

		auto leftObjects = std::vector<BVHPrimitiveInfo>(begin, target);
		auto rightObjects = std::vector<BVHPrimitiveInfo>(target, end);

		node->left = RecursiveBuild(leftObjects);
		node->right = RecursiveBuild(rightObjects);
		node->bounds = UnionBox(node->left->bounds, node->right->bounds);
	}
	return node;
}

BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>>& p)
{
	primitives = std::move(p);
	if (primitives.empty()) return;
	int totalNodes = 0;

	std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
	for (size_t i = 0; i < primitives.size(); i++)
	{
		//primitiveInfo[i] = { i, CreateBoundingBox(primitives[i].get()) };
		primitiveInfo[i].primitiveIndex = i;
		primitiveInfo[i].boundbox = CreateBoundingBox(primitives[i].get());
		primitiveInfo[i].centroid = 0.5f * primitiveInfo[i].boundbox.pMin + 0.5f * primitiveInfo[i].boundbox.pMax;
	}

	std::vector<std::shared_ptr<Primitive>> orderedPrims;
	orderedPrims.reserve(primitives.size());

	BVHBuildNode* root;
	root = RecursiveBuild(primitiveInfo); // SAH


	//int depth = glm::ceil(glm::log2((float)primitives.size())) + 1;
	//LBVHNodes.resize(glm::pow(2, depth) - 1);

	FlattenBVH(root);

	ReleaseTreeMemory(root);
}

void DFS(BVHBuildNode* root)
{
	if (root == nullptr) return;
	if (root->left != nullptr) DFS(root->left);
	if (root->right != nullptr) DFS(root->right);
	delete root;
}
void BVHAccel::ReleaseTreeMemory(BVHBuildNode* root)
{
	DFS(root);
	return;
}


void BVHAccel::FlattenBVH(BVHBuildNode* root)
{
	if (root == nullptr) return;
	int currentIdx = LBVHNodes.size();

	LinearBVHNode node;
	node.bounds = root->bounds;
	node.primitiveIndex = root->primitiveIdx;
	if (root->left != nullptr)
	{
		node.leftRootIdx = 2 * currentIdx + 1;
	}

	if (root->right != nullptr)
	{
		node.rightRootIdx = 2 * currentIdx + 2;
	}
	LBVHNodes.push_back(node);
	if(root->left) FlattenBVH(root->left);
	if (root->right) FlattenBVH(root->right);
}


std::vector<LinearBVHNodeGPU> BVHAccel::GetLinearBVHGPUNode()
{
	std::vector<LinearBVHNodeGPU> res;
	for (int i = 0; i < LBVHNodes.size(); i++)
	{
		LinearBVHNodeGPU nodeGPU;
		nodeGPU.leftNodeIdx = LBVHNodes[i].leftRootIdx;
		nodeGPU.rightNodeIdx = LBVHNodes[i].rightRootIdx;
		nodeGPU.pMax = LBVHNodes[i].bounds.pMax;
		nodeGPU.pMin = LBVHNodes[i].bounds.pMin;
		nodeGPU.primitiveIdx = LBVHNodes[i].primitiveIndex;
		res.push_back(nodeGPU);
	}
	return res;
}


