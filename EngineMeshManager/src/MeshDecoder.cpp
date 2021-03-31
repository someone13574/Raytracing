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

			Mesh::Vertex currentVertices[3];
			float normal[3] = { 0, 0, 0 };

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
									normal[tokenIndex] = std::stof(intermediate);
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
										}
										tokenIndex++;
									}

									currentVertices[i].UV[0] = 0;
									currentVertices[i].UV[1] = 0;
								}
							}
							else
							{
								currentMesh->AddTriangle(currentVertices, normal);
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