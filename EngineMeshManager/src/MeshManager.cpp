#include "MeshManager.h"
#include "EngineLogger.h"

#include <string>
#include <ctime>

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

				float timeStart = float(clock()) / CLOCKS_PER_SEC;
				std::vector<Mesh> readFileResult = MeshDecoder::ReadAsciiStl(path);
				DEBUGLOG((float(clock()) / CLOCKS_PER_SEC) - timeStart);

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

	std::vector<Mesh::Vertex> MeshManager::GetVertexArray()
	{
		upToDate = true;

		std::vector<Mesh::Vertex> vertices;
		for (int i = 0; i < meshes.size(); i++)
		{
			vertices.insert(vertices.end(), meshes[i].vertices.begin(), meshes[i].vertices.end());
		}

		return vertices;
	}
}