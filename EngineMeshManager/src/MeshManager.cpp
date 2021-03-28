#include "MeshManager.h"
#include "EngineLogger.h"

#include <string>

namespace MeshManagement
{
	MeshManager::MeshManager()
	{

	}

	void MeshManager::ReadMeshFile(const char* path)
	{
		std::string pathstr = std::string(path);
		size_t extensionIndex = pathstr.find_last_of('.');
		if (extensionIndex != std::string::npos)
		{
			pathstr.erase(0, extensionIndex + 1);
			if (pathstr == "stl")
			{
				std::vector<Mesh> readFileResult = MeshDecoder::ReadAsciiStl(path);
				meshes.insert(meshes.end(), readFileResult.begin(), readFileResult.end());
				upToDate = false;
			}
			else
			{
				DEBUGERROR("ReadMeshFile Failed: Unknown File Extension")
			}
		}
	}

	Mesh MeshManager::GetMesh(uint16_t index)
	{
		return meshes[index];
	}

	bool MeshManager::IsUpToDate() const
	{
		return upToDate;
	}

	std::vector<Mesh::Triangle> MeshManager::GetTriangleArray()
	{
		upToDate = true;

		std::vector<Mesh::Triangle> triangles;
		for (int i = 0; i < meshes.size(); i++)
		{
			triangles.insert(triangles.end(), meshes[i].triangles.begin(), meshes[i].triangles.end());
		}

		return triangles;
	}
}