#include "Mesh.h"
#include "EngineLogger.h"

Mesh::Mesh(std::string name) : meshName(name) {}

void Mesh::AddTriangle(Mesh::Vertex triangleVertices[3], float normal[3])
{
	Triangle triangle;
	UncompressedTriangle uncompressedTriangle;

	unsigned int indices1 = 0x00000000; //The first 32 bit number containing the first and second vertices
	unsigned int indices2 = 0x00000000; //The second 32 bit number containing the second and third vertices

	for (unsigned int vert = 0; vert < 3; vert++) //Loop through triangle's vertices
	{
		unsigned int vertexIndex = 0;
		bool foundDuplicate = false;
		if (vertices.size() > 0)
		{
			for (long long int index = (long long int)(vertices.size() - 1); index >= 0; index--) //Loop through all current vertices starting at top
			{
				bool isDuplicate = (abs(vertices[index].position[0] - triangleVertices[vert].position[0]) < 0.1 &&
					abs(vertices[index].position[1] - triangleVertices[vert].position[1]) < 0.1 &&
					abs(vertices[index].position[2] - triangleVertices[vert].position[2]) < 0.1);

				if (isDuplicate) //Test if this vertex already exists in a different triangle
				{
					vertexIndex = (unsigned int)index;
					foundDuplicate = true;
					break;
				}
			}
		}
		if (!foundDuplicate) //Add vertex to array if it doesn't already exist
		{
			vertexIndex = (unsigned int)vertices.size();
			vertices.push_back(triangleVertices[vert]);
		}
		if (vert == 0)
		{
			indices1 = indices1 | (vertexIndex & 0x000fffff); //Mask first index
		}
		else if (vert == 1)
		{
			indices1 = indices1 | ((vertexIndex << 20) & 0xfff00000); //Mask and shift second index
			indices2 = indices2 | ((vertexIndex >> 12) & 0x000000ff);
		}
		else
		{
			indices2 = indices2 | ((vertexIndex << 8) & 0x0fffff00); //Mask and shift third index
		}
	}

	triangle.indices1 = indices1;
	triangle.indices2 = indices2;
	triangle.normal[0] = normal[0];
	triangle.normal[1] = normal[1];
	triangle.normal[2] = normal[2];

	uncompressedTriangle.vertices[0] = triangleVertices[0];
	uncompressedTriangle.vertices[1] = triangleVertices[1];
	uncompressedTriangle.vertices[2] = triangleVertices[2];

	triangles.push_back(triangle);
	uncompressedTriangles.push_back(uncompressedTriangle);
}