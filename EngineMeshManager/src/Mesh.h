#pragma once

#include <string>
#include <vector>

class __declspec(dllexport) Mesh
{
public:
	Mesh(std::string name);
public:
	struct Vertex
	{
		float position[3];
		float UV[2];
	};
	struct Triangle
	{
		unsigned int indices1;
		unsigned int indices2;
		float normal[3];
	};
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
		unsigned int triangleIndex;

		unsigned int parentIndex;
		unsigned int childAIndex;
		unsigned int childBIndex;

		AABB aabb;
		int isLeaf;
	};
	struct UncompressedTriangle
	{
		Vertex vertices[3];
		float normal[3];
	};
public:
	void AddTriangle(Vertex vertices[3], float normal[3]);
	unsigned int GetMaxStackSize();
public:
	bool completed = false;
public:
	unsigned int rootIndex = 4294967295;
#pragma warning(push)
#pragma warning(disable:4251)
	std::string meshName;
	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
	std::vector<UncompressedTriangle> uncompressedTriangles;
	std::vector<Node> nodeHierarchy;
#pragma warning(pop)
};