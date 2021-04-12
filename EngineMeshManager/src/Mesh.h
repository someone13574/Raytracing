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
		float normal[3];
		float UV[2];
	};
	struct Triangle
	{
		unsigned int indices1;
		unsigned int indices2;
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
	struct LinkedNode
	{
		unsigned int triangleIndex;

		unsigned int hitLink;
		unsigned int missLink;

		AABB aabb;
		int isLeaf;
	};
public:
	void AddTriangle(Vertex vertices[3]);
public:
	bool completed = false;
public:
	unsigned int rootIndex = 4294967295;
#pragma warning(push)
#pragma warning(disable:4251)
	std::vector<LinkedNode> GetLinkedNodeHierarchy();

	std::string meshName;
	std::vector<Vertex> vertices;
	std::vector<unsigned short int> vertexUsedCount;
	std::vector<Triangle> triangles;
	std::vector<Node> nodeHierarchy;
#pragma warning(pop)
};