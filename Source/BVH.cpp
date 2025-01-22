#include <BVH.h>
#include <stack>
#include <algorithm>
#include <log.h>

using namespace BVHBuildTool;

const glm::vec3 eps(0.0001);

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

std::shared_ptr<BVHBuildNode> BVHAccel::RecursiveBuild(std::vector<BVHPrimitiveInfo>& primitiveInfo, int start,int end,int* totalNodes ,std::vector<std::shared_ptr<Primitive>>& orderedPrims)
{
	int maxPrimsInNode = 4;

	std::shared_ptr<BVHBuildNode> node = std::make_shared<BVHBuildNode>();
	(*totalNodes)++;
	// Compute bounds of all primitives in BVH node
	Boundbox bounds;
	for (int i = start; i < end; ++i)
		bounds = BVHBuildTool::UnionBox(bounds, primitiveInfo[i].boundbox);

	int nPrimitives = end - start;
	if (nPrimitives == 1)
	{
		// Create leaf _BVHBuildNode_
		int firstPrimOffset = orderedPrims.size();
		for (int i = start; i < end; ++i) {
			int primNum = primitiveInfo[i].primitiveNumber;
			orderedPrims.push_back(primitives[i]);
		}
		node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
		return node;
	}
	else
	{
		// Compute bound of primitive centroids, choose split dimension _dim_
		Boundbox centroidBound;
		for (int i = start; i < end; i++)
			centroidBound = UnionBox(centroidBound, primitiveInfo[i].centroid);

		int dim = centroidBound.MaxExtent();
		
		int mid = (start + end) / 2;
		if (centroidBound.pMin == centroidBound.pMax)
		{
			//create leaf
			int firstPrimOffset = orderedPrims.size();
			for (int i = start; i < end; ++i) {
				int primNum = primitiveInfo[i].primitiveNumber;
				orderedPrims.push_back(primitives[primNum]);
			}
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else
		{
			//Begin building SAH
			if (nPrimitives <= 2)
			{   
				// Partition primitives into equally-sized subsets
				mid = (start + end) / 2;
				std::nth_element(&primitiveInfo[start], &primitiveInfo[mid], &primitiveInfo[end - 1] + 1, 
					[dim](const BVHPrimitiveInfo& a,const BVHPrimitiveInfo& b) {
						return a.centroid[dim] <b.centroid[dim];});
			}
			else
			{
				// Allocate _BucketInfo_ for SAH partition buckets
				constexpr int nBuckets = 12;
				BucketInfo buckets[nBuckets];
				for (int i = start; i < end; i++)
				{
					int b = nBuckets *centroidBound.Offset(primitiveInfo[i].centroid)[dim];
					if (b == nBuckets) b = nBuckets - 1;
					buckets[b].count++;
					buckets[b].bounds =UnionBox(buckets[b].bounds, primitiveInfo[i].boundbox);
				}

				float cost[nBuckets - 1];
				for (int i = 0; i < nBuckets - 1; i++)
				{
					Boundbox b0, b1;
					int count0 = 0;
					int count1 = 0;
					for (int j = 0; j <= i; j++)
					{
						b0 = UnionBox(b0, buckets[j].bounds);
						count0 += buckets[j].count;
					}

					for (int j = i + 1; j < nBuckets; j++)
					{
						b1 = UnionBox(b1, buckets[j].bounds);
						count1 += buckets[j].count;
					}

					cost[i] = 1 +
						(count0 * b0.SurfaceArea() +
							count1 * b1.SurfaceArea()) /
						bounds.SurfaceArea();
				}

				float minCost = cost[0];
				int minCostSplitBucket = 0;
				for (int i = 1; i < nBuckets - 1; ++i)
				{
					if (cost[i] < minCost)
					{
						minCost = cost[i];
						minCostSplitBucket = i;
					}
				}

				// Either create leaf or split primitives at selected SAH
				// bucket
				float leafCost = nPrimitives;
				if (nPrimitives > maxPrimsInNode || minCost < leafCost)
				{
					BVHPrimitiveInfo* pmid = std::partition(
						&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
						[=](const BVHPrimitiveInfo& pi) {
							int b = nBuckets *
								centroidBound.Offset(pi.centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							return b <= minCostSplitBucket;
						});
					mid = pmid - &primitiveInfo[0];
				}
				else
				{
					// Create leaf _BVHBuildNode_
					int firstPrimOffset = orderedPrims.size();
					for (int i = start; i < end; ++i) {
						int primNum = primitiveInfo[i].primitiveNumber;
						orderedPrims.push_back(primitives[primNum]);
					}
					node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
					return node;
				}

				node->InitInterior(dim,
					RecursiveBuild(primitiveInfo, start, mid,
						totalNodes, orderedPrims),
					RecursiveBuild(primitiveInfo, mid, end,
						totalNodes, orderedPrims));
			}
		}
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
		primitiveInfo[i].primitiveNumber = i;
		primitiveInfo[i].boundbox = CreateBoundingBox(primitives[i].get());
		primitiveInfo[i].centroid = 0.5f * primitiveInfo[i].boundbox.pMin + 0.5f * primitiveInfo[i].boundbox.pMax;
	}

	std::vector<std::shared_ptr<Primitive>> orderedPrims;
	orderedPrims.reserve(primitives.size());

	std::shared_ptr<BVHBuildNode> root;
	root = RecursiveBuild(primitiveInfo, 0, primitiveInfo.size(),&totalNodes,orderedPrims); // SAH

	primitives.swap(orderedPrims);
	LBVHNodes.resize(totalNodes);
	std::generate(LBVHNodes.begin(),LBVHNodes.end(),
		[]() { return std::make_shared<LinearBVHNode>(); }
	);

	int offset = 0;
	FlattenBVH(root.get(), &offset);
}

int BVHAccel::FlattenBVH(BVHBuildNode* node, int* offset)
{
	std::shared_ptr<LinearBVHNode> linearNode = LBVHNodes[*offset];
	linearNode->bounds = node->bounds;
	int myOffset = (*offset)++;
	if (node->nPrimitives > 0)
	{
		linearNode->primitivesOffset = node->firstPrimOffset;
		linearNode->nPrimitives = node->nPrimitives;
	}
	else {
		// Create interior flattened BVH node
		linearNode->axis = node->splitAxis;
		linearNode->nPrimitives = 0;
		FlattenBVH(node->children[0], offset);
		linearNode->secondChildOffset =	FlattenBVH(node->children[1], offset);
	}
	return myOffset;
}


std::vector<LinearBVHNodeGPU> BVHAccel::GetLinearBVHGPUNode()
{
	std::vector<LinearBVHNodeGPU> res;

	for (auto& LBVHNode : LBVHNodes)
	{
		LinearBVHNodeGPU linearGPU;
		linearGPU.pMax = LBVHNode->bounds.pMax;
		linearGPU.pMin = LBVHNode->bounds.pMin;
		linearGPU.nPrimitives = LBVHNode->nPrimitives;
		linearGPU.primitivesOffset = LBVHNode->primitivesOffset;
		linearGPU.secondChildOffset = LBVHNode->secondChildOffset;
		linearGPU.axis = LBVHNode->axis;
	}
	return res;
}


