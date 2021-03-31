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
	struct UncompressedTriangle
	{
		Vertex vertices[3];
		float normal[3];
	};
public:
	void AddTriangle(Vertex vertices[3], float normal[3]);
public:
	bool completed = false;
public:
#pragma warning(push)
#pragma warning(disable:4251)
	std::string meshName;
	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
	std::vector<UncompressedTriangle> uncompressedTriangles;
#pragma warning(pop)
};