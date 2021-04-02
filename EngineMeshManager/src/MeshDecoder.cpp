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
							for (unsigned int vert = 0; vert < (unsigned int)currentMesh->vertices.size(); vert++) //Loop through all vertices and divide sum of triangle normals by count of triangle normals
							{
								unsigned short int numberOfUses = currentMesh->vertexUsedCount[vert];

								currentMesh->vertices[vert].normal[0] /= (float)numberOfUses;
								currentMesh->vertices[vert].normal[1] /= (float)numberOfUses;
								currentMesh->vertices[vert].normal[2] /= (float)numberOfUses;

								//Normalize
								double magnitude = sqrt((currentMesh->vertices[vert].normal[0] * currentMesh->vertices[vert].normal[0]) + (currentMesh->vertices[vert].normal[1] * currentMesh->vertices[vert].normal[1]) + (currentMesh->vertices[vert].normal[2] * currentMesh->vertices[vert].normal[2]));
								currentMesh->vertices[vert].normal[0] /= (float)magnitude;
								currentMesh->vertices[vert].normal[1] /= (float)magnitude;
								currentMesh->vertices[vert].normal[2] /= (float)magnitude;
							}

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

									currentVertices[i].normal[0] = normal[0];
									currentVertices[i].normal[1] = normal[1];
									currentVertices[i].normal[2] = normal[2];
								}
							}
							else
							{
								currentMesh->AddTriangle(currentVertices);
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

	Mesh MeshDecoder::ReadObj(const char* path)
	{
		std::ifstream file;
		file.open(path);

		if (file.is_open())
		{
			struct VertexPosition
			{
				float position[3];
			};
			struct VertexTextureCoord
			{
				float uv[2];
			};
			struct VertexNormal
			{
				float normal[3];
			};
			std::vector<VertexPosition> vertexPositions;
			std::vector<VertexTextureCoord> vertexTextureCoords;
			std::vector<VertexNormal> vertexNormals;

			Mesh mesh = Mesh("Empty mesh");
			std::string currentLine;

			while (std::getline(file, currentLine))
			{
				if (currentLine.substr(0, 1) == "#") //Test if line is a comment
				{
					continue;
				}

				if (currentLine.find("mtllib") == 0) //Test if line contains mtl file directory
				{
					continue;
				}

				if (currentLine.find("o") == 0)
				{
					std::size_t spaceIndex = currentLine.find_first_of(" ");
					if (spaceIndex != std::string::npos)
					{
						currentLine.erase(0, spaceIndex + 1);
						mesh = Mesh(currentLine);
					}
					else
					{
						mesh = Mesh("New Mesh");
					}
					continue;
				}

				if (currentLine.find("v ") == 0)
				{
					std::size_t spaceIndex = currentLine.find(" ");
					if (spaceIndex != std::string::npos)
					{
						VertexPosition currentPosition;

						std::stringstream ss(currentLine);
						std::string intermediate;
						int tokenIndex = -1;
						while (std::getline(ss, intermediate, ' '))
						{
							if (tokenIndex >= 0)
							{
								currentPosition.position[tokenIndex] = std::stof(intermediate);
							}
							tokenIndex++;
						}

						vertexPositions.push_back(currentPosition);
					}
					continue;
				}

				if (currentLine.find("vt") == 0)
				{
					std::size_t spaceIndex = currentLine.find(" ");
					if (spaceIndex != std::string::npos)
					{
						VertexTextureCoord currentTextureCoord;

						std::stringstream ss(currentLine);
						std::string intermediate;
						int tokenIndex = -1;
						while (std::getline(ss, intermediate, ' '))
						{
							if (tokenIndex >= 0 && tokenIndex < 2)
							{
								currentTextureCoord.uv[tokenIndex] = std::stof(intermediate);
							}
							tokenIndex++;
						}
						
						vertexTextureCoords.push_back(currentTextureCoord);
					}
					continue;
				}

				if (currentLine.find("vn") == 0)
				{
					std::size_t spaceIndex = currentLine.find(" ");
					if (spaceIndex != std::string::npos)
					{
						VertexNormal currentNormal;

						std::stringstream ss(currentLine);
						std::string intermediate;
						int tokenIndex = -1;
						while (std::getline(ss, intermediate, ' '))
						{
							if (tokenIndex >= 0 && tokenIndex < 3)
							{
								currentNormal.normal[tokenIndex] = std::stof(intermediate);
							}
							tokenIndex++;
						}
						
						vertexNormals.push_back(currentNormal);
					}
					continue;
				}

				if (currentLine.find("f ") == 0)
				{
					std::size_t spaceIndex = currentLine.find(" ");
					if (spaceIndex != std::string::npos)
					{
						Mesh::Vertex currentVertices[3];

						std::stringstream ss(currentLine);
						std::string intermediate;
						int tokenIndex = -1;
						while (std::getline(ss, intermediate, ' '))
						{
							if (tokenIndex >= 0 && tokenIndex < 3)
							{
								std::stringstream ss2(intermediate);
								std::string intermediate2;
								int attributeIndex = 0;

								while (std::getline(ss2, intermediate2, '/'))
								{
									switch (attributeIndex)
									{
									case 0:
									{
										currentVertices[tokenIndex].position[0] = vertexPositions[std::stoi(intermediate2) - 1].position[0];
										currentVertices[tokenIndex].position[1] = vertexPositions[std::stoi(intermediate2) - 1].position[1];
										currentVertices[tokenIndex].position[2] = vertexPositions[std::stoi(intermediate2) - 1].position[2];
										break;
									}
									case 1:
									{
										currentVertices[tokenIndex].UV[0] = vertexTextureCoords[std::stoi(intermediate2) - 1].uv[0];
										currentVertices[tokenIndex].UV[1] = vertexTextureCoords[std::stoi(intermediate2) - 1].uv[1];
										break;
									}
									case 2:
									{
										currentVertices[tokenIndex].normal[0] = vertexNormals[std::stoi(intermediate2) - 1].normal[0];
										currentVertices[tokenIndex].normal[1] = vertexNormals[std::stoi(intermediate2) - 1].normal[1];
										currentVertices[tokenIndex].normal[2] = vertexNormals[std::stoi(intermediate2) - 1].normal[2];
										break;
									}
									}

									attributeIndex++;
								}


							}
							tokenIndex++;
						}

						mesh.AddTriangle(currentVertices);
					}
					continue;
				}
			}

			for (unsigned int vert = 0; vert < (unsigned int)mesh.vertices.size(); vert++) //Loop through all vertices and divide sum of triangle normals by count of triangle normals
			{
				unsigned short int numberOfUses = mesh.vertexUsedCount[vert];

				mesh.vertices[vert].normal[0] /= (float)numberOfUses;
				mesh.vertices[vert].normal[1] /= (float)numberOfUses;
				mesh.vertices[vert].normal[2] /= (float)numberOfUses;

				//Normalize
				double magnitude = sqrt((mesh.vertices[vert].normal[0] * mesh.vertices[vert].normal[0]) + (mesh.vertices[vert].normal[1] * mesh.vertices[vert].normal[1]) + (mesh.vertices[vert].normal[2] * mesh.vertices[vert].normal[2]));
				mesh.vertices[vert].normal[0] /= (float)magnitude;
				mesh.vertices[vert].normal[1] /= (float)magnitude;
				mesh.vertices[vert].normal[2] /= (float)magnitude;
			}

			mesh.completed = true;

			file.close();

			return mesh;
		}

		return Mesh("Empty Mesh");
	}
}