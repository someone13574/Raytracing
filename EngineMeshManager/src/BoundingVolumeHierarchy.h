#pragma once

#include "Mesh.h"

class __declspec(dllexport) BoundingVolumeHierarchy
{
public:
	struct AABB
	{
		float ax;
		float ay;
		float az;
		float bx;
		float by;
		float bz;
	};
	struct Node
	{
		uint32_t triangleIndex;

		uint32_t parentIndex;
		uint32_t childAIndex;
		uint32_t childBIndex;

		AABB aabb;
		int isLeaf;
	};
	struct Tree
	{
		uint32_t rootIndex;
		uint32_t nodeCount;
		uint16_t triangleCount;
		std::vector<Mesh::UncompressedTriangle> triangles;
		std::vector<Node> nodeHierarchy;
	};
public:
	BoundingVolumeHierarchy();
	BoundingVolumeHierarchy(Mesh mesh);
	void ReadMesh(Mesh mesh);
	void ReadMeshOptimized(Mesh mesh);
	Tree GetTreeData() const;
	unsigned int GetMaxStackSize();

	float ComputeTreeCost();
	float SurfaceArea(AABB aabb);
private:
	uint32_t AllocateNode(Node node);
	AABB TriangleToAABB(Mesh::UncompressedTriangle triangle);
	AABB JoinAABB(AABB a, AABB b);
	void Rotate(unsigned int index);
private:
	Mesh mesh;
	uint32_t rootIndex = 4294967295;
#pragma warning(push)
#pragma warning(disable:4251)
	std::vector<Node> nodeHierarchy;
#pragma warning(pop)

	Tree tree;
};