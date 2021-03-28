#pragma once

#include "MeshDecoder.h"
#include "Mesh.h"

#include <vector>

namespace MeshManagement
{
	class __declspec(dllexport) MeshManager
	{
	public:
		MeshManager();
	public:
		void ReadMeshFile(const char* path);
		bool IsUpToDate() const;
		std::vector<Mesh::Triangle> GetTriangleArray();
		Mesh GetMesh(uint16_t index);
	private:
		bool upToDate = false;
#pragma warning(push)
#pragma warning(disable:4251)
		std::vector<Mesh> meshes;
#pragma warning(pop)
	};
}