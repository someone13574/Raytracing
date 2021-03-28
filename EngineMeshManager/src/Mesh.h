#pragma once

#include <string>
#include <vector>

class __declspec(dllexport) Mesh
{
public:
	Mesh(std::string name);
public:
	struct Triangle
	{
		float normal[3];
		float vertexPositions[3][3];
	};
public:
	bool completed = false;
public:
#pragma warning(push)
#pragma warning(disable:4251)
	std::string meshName;
	std::vector<Triangle> triangles;
#pragma warning(pop)
};