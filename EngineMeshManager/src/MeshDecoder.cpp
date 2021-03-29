#include "MeshDecoder.h"
#include "EngineLogger.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

namespace MeshManagement
{
	std::vector<Mesh> MeshDecoder::ReadAsciiStl(const char* path)
	{
		std::ifstream file;
		file.open(path);

		if (file.is_open())
		{
			std::vector<Mesh> meshArray;
			Mesh* currentMesh = nullptr;
			Mesh::Triangle currentTriangle;
			Mesh::UncompressedTriangle currentUncompressedTriangle;
			Mesh::Vertex currentVertices[3];

			std::string currentLine;

			while (std::getline(file, currentLine))
			{
				//Start and end mesh
				std::size_t solidIndex = currentLine.find("solid");
				std::size_t spaceIndex = currentLine.find_first_of(" ");
				if (solidIndex != std::string::npos && spaceIndex != std::string::npos)
				{
					currentLine.erase(0, spaceIndex + 1);
					if (solidIndex == 0)
					{
						meshArray.push_back(Mesh(currentLine));
						currentMesh = &meshArray.back();
					}
					else
					{
						if (currentMesh != nullptr)
						{
							currentMesh->completed = true;
						}
					}
				}
				else
				{
					std::size_t facetIndex = currentLine.find("facet");
					if (facetIndex != std::string::npos)
					{
						if (facetIndex == 0)
						{
							std::stringstream ss(currentLine);
							std::string intermediate;
							int tokenIndex = -2;
							while (std::getline(ss, intermediate, ' '))
							{
								if (tokenIndex >= 0)
								{
									currentTriangle.normal[tokenIndex] = std::stof(intermediate);
									currentUncompressedTriangle.normal[tokenIndex] = std::stof(intermediate);
								}
								tokenIndex++;
							}
						}
					}
					else
					{
						std::size_t loopIndex = currentLine.find("loop");
						if (loopIndex != std::string::npos)
						{
							if (loopIndex == 6)
							{
								for (int i = 0; i < 3; i++)
								{
									std::getline(file, currentLine);
									std::stringstream ss(currentLine);
									std::string intermediate;
									int tokenIndex = -1;
									while (std::getline(ss, intermediate, ' '))
									{
										if (tokenIndex >= 0)
										{
											currentVertices[i].position[tokenIndex] = std::stof(intermediate);
											currentUncompressedTriangle.vertices[i].position[tokenIndex] = std::stof(intermediate);
										}
										tokenIndex++;
									}
								}
							}
							else
							{
								uint64_t vertexIndices[3];
								unsigned int indices1 = 0x00000000;
								unsigned int indices2 = 0x00000000;
								for (unsigned int i = 0; i < 3; i++)
								{
									auto it = std::find(currentMesh->vertices.begin(), currentMesh->vertices.end(), currentVertices[i]);
									if (it != currentMesh->vertices.end()) //Test if the mesh already has this vertex
									{
										vertexIndices[i] = it - currentMesh->vertices.begin();
									}
									else
									{
										currentMesh->vertices.push_back(currentVertices[i]);
										vertexIndices[i] = currentMesh->vertices.size() - 1;
									}
									if (i == 0)
									{
										indices1 = indices1 | (vertexIndices[i] & 0x000fffff);
									}
									else if (i == 1)
									{
										indices1 = indices1 | ((vertexIndices[i] << 20) & 0xfff00000);
										indices2 = indices2 | ((vertexIndices[i] >> 12) & 0x000000ff);
									}
									else if (i == 2)
									{
										indices2 = indices2 | ((vertexIndices[i] << 8) & 0x0fffff00);
									}
								}

								currentTriangle.indices1 = indices1;
								currentTriangle.indices2 = indices2;

								currentMesh->triangles.push_back(currentTriangle);
								currentMesh->uncompressedTriangles.push_back(currentUncompressedTriangle);
							}
						}
					}
				}
			}

			file.close();

			return meshArray;
		}
		else
		{
			DEBUGLOG("Failed to open file.");
			std::vector<Mesh> errorMeshArray;
			errorMeshArray.push_back(Mesh("Error opening file."));
			return errorMeshArray;
		}
	}
}