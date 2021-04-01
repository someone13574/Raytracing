#include "Mesh.h"

#include <stack>

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

	//Add to bounding volume hierarchy
	{
		AABB aabb; //Get bounding volume of triangle
		aabb.ax = fminf(triangleVertices[0].position[0], fminf(triangleVertices[1].position[0], triangleVertices[2].position[0]));
		aabb.ay = fminf(triangleVertices[0].position[1], fminf(triangleVertices[1].position[1], triangleVertices[2].position[1]));
		aabb.az = fminf(triangleVertices[0].position[2], fminf(triangleVertices[1].position[2], triangleVertices[2].position[2]));
		aabb.bx = fmaxf(triangleVertices[0].position[0], fmaxf(triangleVertices[1].position[0], triangleVertices[2].position[0]));
		aabb.by = fmaxf(triangleVertices[0].position[1], fmaxf(triangleVertices[1].position[1], triangleVertices[2].position[1]));
		aabb.bz = fmaxf(triangleVertices[0].position[2], fmaxf(triangleVertices[1].position[2], triangleVertices[2].position[2]));

		Node node; //Create new leaf node
		node.triangleIndex = (unsigned int)triangles.size() - 1;
		node.aabb = aabb;
		node.isLeaf = 1;
		node.parentIndex = 0;
		node.childAIndex = 4294967295;
		node.childBIndex = 4294967295;

		unsigned int nodeIndex = (unsigned int)nodeHierarchy.size(); //Allocate new node in node hierarchy
		nodeHierarchy.push_back(node);

		if (nodeIndex == 0) //If the new node is the only node, set as root index
		{
			rootIndex = nodeIndex;
			nodeHierarchy[nodeIndex].parentIndex = 4294967295;
			return;
		}

		//Find best pair
		unsigned int sibling = rootIndex;
		std::stack<unsigned int> priorityQueue;
		double bestCost = INFINITY;
		priorityQueue.push(rootIndex);
		while (priorityQueue.size() > 0)
		{
			unsigned int currentNodeIndex = priorityQueue.top();
			Node* currentNode = &nodeHierarchy[currentNodeIndex]; //Get node at the top of priority queue
			priorityQueue.pop(); //Remove top node from queue

			//Direct cost
			AABB joined;
			joined.ax = fminf(aabb.ax, currentNode->aabb.ax);
			joined.ay = fminf(aabb.ay, currentNode->aabb.ay);
			joined.az = fminf(aabb.az, currentNode->aabb.az);

			joined.bx = fmaxf(aabb.bx, currentNode->aabb.bx);
			joined.by = fmaxf(aabb.by, currentNode->aabb.by);
			joined.bz = fmaxf(aabb.bz, currentNode->aabb.bz);

			float d[3] = { joined.bx - joined.ax,  joined.by - joined.ay,  joined.bz - joined.az };
			double directCost = 2 * (d[0] * d[1] + d[1] * d[2] + d[2] * d[0]);

			//Indirect cost
			double indirectCost = 0;
			unsigned int currentIndex = currentNode->parentIndex;

			while (currentIndex != 4294967295)
			{
				joined.ax = fminf(joined.ax, nodeHierarchy[currentIndex].aabb.ax);
				joined.ay = fminf(joined.ay, nodeHierarchy[currentIndex].aabb.ay);
				joined.az = fminf(joined.az, nodeHierarchy[currentIndex].aabb.az);

				joined.bx = fmaxf(joined.bx, nodeHierarchy[currentIndex].aabb.bx);
				joined.by = fmaxf(joined.by, nodeHierarchy[currentIndex].aabb.by);
				joined.bz = fmaxf(joined.bz, nodeHierarchy[currentIndex].aabb.bz);

				float d1[3] = { joined.bx - joined.ax,  joined.by - joined.ay,  joined.bz - joined.az };
				float d2[3] = { nodeHierarchy[currentIndex].aabb.bx - nodeHierarchy[currentIndex].aabb.ax,  nodeHierarchy[currentIndex].aabb.by - nodeHierarchy[currentIndex].aabb.ay,  nodeHierarchy[currentIndex].aabb.bz - nodeHierarchy[currentIndex].aabb.az };
				double directCost = 2 * (d[0] * d[1] + d[1] * d[2] + d[2] * d[0]);

				indirectCost += (2 * (d1[0] * d1[1] + d1[1] * d1[2] + d1[2] * d1[0])) - (2 * (d2[0] * d2[1] + d2[1] * d2[2] + d2[2] * d2[0]));
				currentIndex = nodeHierarchy[currentIndex].parentIndex;
			}

			//Update and add to queue
			double totalCost = directCost + indirectCost;
			if (totalCost < bestCost)
			{
				bestCost = totalCost;
				sibling = currentNodeIndex;

				if (currentNode->childAIndex != 4294967295)
				{
					float clowd[3] = { aabb.bx - aabb.ax,  aabb.by - aabb.ay,  aabb.bz - aabb.az };
					double clow = 2 * (clowd[0] * clowd[1] + clowd[1] * clowd[2] + clowd[2] * clowd[0]);

					joined = currentNode->aabb;
					currentIndex = currentNode->parentIndex;
					while (currentIndex != 4294967295)
					{
						joined.ax = fminf(joined.ax, nodeHierarchy[currentIndex].aabb.ax);
						joined.ay = fminf(joined.ay, nodeHierarchy[currentIndex].aabb.ay);
						joined.az = fminf(joined.az, nodeHierarchy[currentIndex].aabb.az);

						joined.bx = fmaxf(joined.bx, nodeHierarchy[currentIndex].aabb.bx);
						joined.by = fmaxf(joined.by, nodeHierarchy[currentIndex].aabb.by);
						joined.bz = fmaxf(joined.bz, nodeHierarchy[currentIndex].aabb.bz);

						float clowd1[3] = { joined.bx - joined.ax,  joined.by - joined.ay,  joined.bz - joined.az };
						float clowd2[3] = { nodeHierarchy[currentIndex].aabb.bx - nodeHierarchy[currentIndex].aabb.ax,  nodeHierarchy[currentIndex].aabb.by - nodeHierarchy[currentIndex].aabb.ay,  nodeHierarchy[currentIndex].aabb.bz - nodeHierarchy[currentIndex].aabb.az };

						clow += (2 * (clowd1[0] * clowd1[1] + clowd1[1] * clowd1[2] + clowd1[2] * clowd1[0])) - (2 * (clowd2[0] * clowd2[1] + clowd2[1] * clowd2[2] + clowd2[2] * clowd2[0]));
						currentIndex = nodeHierarchy[currentIndex].parentIndex;
					}

					//test if clow is lower then best cost
					if (clow < bestCost)
					{
						priorityQueue.push(currentNode->childAIndex);
						priorityQueue.push(currentNode->childBIndex);
					}
				}
			}
		}

		//Create parent
		unsigned int oldParent = nodeHierarchy[sibling].parentIndex;

		AABB newParentAABB;
		newParentAABB.ax = fminf(aabb.ax, nodeHierarchy[sibling].aabb.ax);
		newParentAABB.ay = fminf(aabb.ay, nodeHierarchy[sibling].aabb.ay);
		newParentAABB.az = fminf(aabb.az, nodeHierarchy[sibling].aabb.az);
		newParentAABB.bx = fmaxf(aabb.bx, nodeHierarchy[sibling].aabb.bx);
		newParentAABB.by = fmaxf(aabb.by, nodeHierarchy[sibling].aabb.by);
		newParentAABB.bz = fmaxf(aabb.bz, nodeHierarchy[sibling].aabb.bz);

		unsigned int newParent = (unsigned int)nodeHierarchy.size(); //Allocate parent node in node hierarchy
		nodeHierarchy.push_back({ 0, oldParent, 0, 0, newParentAABB, 0 });

		if (oldParent == 4294967295)
		{
			nodeHierarchy[newParent].childAIndex = sibling;
			nodeHierarchy[newParent].childBIndex = nodeIndex;
			nodeHierarchy[sibling].parentIndex = newParent;
			nodeHierarchy[nodeIndex].parentIndex = newParent;
			rootIndex = newParent;
			nodeHierarchy[newParent].parentIndex = 4294967295;
		}
		else
		{
			if (nodeHierarchy[oldParent].childAIndex == sibling)
			{
				nodeHierarchy[oldParent].childAIndex = newParent;
			}
			else
			{
				nodeHierarchy[oldParent].childBIndex = newParent;
			}
			nodeHierarchy[newParent].childAIndex = sibling;
			nodeHierarchy[newParent].childBIndex = nodeIndex;
			nodeHierarchy[sibling].parentIndex = newParent;
			nodeHierarchy[nodeIndex].parentIndex = newParent;
		}

		//Refit
		unsigned int currentIndex = nodeHierarchy[nodeIndex].parentIndex;
		while (currentIndex != 4294967295)
		{
			nodeHierarchy[currentIndex].aabb.ax = fminf(nodeHierarchy[nodeHierarchy[currentIndex].childAIndex].aabb.ax, nodeHierarchy[nodeHierarchy[currentIndex].childBIndex].aabb.ax);
			nodeHierarchy[currentIndex].aabb.ay = fminf(nodeHierarchy[nodeHierarchy[currentIndex].childAIndex].aabb.ay, nodeHierarchy[nodeHierarchy[currentIndex].childBIndex].aabb.ay);
			nodeHierarchy[currentIndex].aabb.az = fminf(nodeHierarchy[nodeHierarchy[currentIndex].childAIndex].aabb.az, nodeHierarchy[nodeHierarchy[currentIndex].childBIndex].aabb.az);
			nodeHierarchy[currentIndex].aabb.bx = fmaxf(nodeHierarchy[nodeHierarchy[currentIndex].childAIndex].aabb.bx, nodeHierarchy[nodeHierarchy[currentIndex].childBIndex].aabb.bx);
			nodeHierarchy[currentIndex].aabb.by = fmaxf(nodeHierarchy[nodeHierarchy[currentIndex].childAIndex].aabb.by, nodeHierarchy[nodeHierarchy[currentIndex].childBIndex].aabb.by);
			nodeHierarchy[currentIndex].aabb.bz = fmaxf(nodeHierarchy[nodeHierarchy[currentIndex].childAIndex].aabb.bz, nodeHierarchy[nodeHierarchy[currentIndex].childBIndex].aabb.bz);

			
			{
				Node nodeToRotate = nodeHierarchy[currentIndex];

				//Test if node has parent
				if (nodeToRotate.parentIndex != 4294967295)
				{
					unsigned int parentIndex = nodeToRotate.parentIndex;
					Node parentNode = nodeHierarchy[parentIndex];

					//Test if node has grand parent
					if (parentNode.parentIndex != 4294967295)
					{
						unsigned int grandParentIndex = parentNode.parentIndex;
						Node grandParentNode = nodeHierarchy[grandParentIndex];

						//Find uncle node
						unsigned int uncleIndex;
						Node uncleNode;
						if (grandParentNode.childAIndex != nodeToRotate.parentIndex)
						{
							uncleIndex = grandParentNode.childAIndex;
							uncleNode = nodeHierarchy[uncleIndex];

							//Swap and update child and parent indices
							uncleNode.parentIndex = parentIndex;
							nodeToRotate.parentIndex = grandParentIndex;
							grandParentNode.childAIndex = currentIndex;
							if (parentNode.childAIndex == currentIndex)
							{
								parentNode.childAIndex = uncleIndex;
							}
							else
							{
								parentNode.childBIndex = uncleIndex;
							}

							//Refit parent aabb
							parentNode.aabb.ax = fminf(nodeHierarchy[parentNode.childAIndex].aabb.ax, nodeHierarchy[parentNode.childBIndex].aabb.ax);
							parentNode.aabb.ay = fminf(nodeHierarchy[parentNode.childAIndex].aabb.ay, nodeHierarchy[parentNode.childBIndex].aabb.ay);
							parentNode.aabb.az = fminf(nodeHierarchy[parentNode.childAIndex].aabb.az, nodeHierarchy[parentNode.childBIndex].aabb.az);
							parentNode.aabb.bx = fmaxf(nodeHierarchy[parentNode.childAIndex].aabb.bx, nodeHierarchy[parentNode.childBIndex].aabb.bx);
							parentNode.aabb.by = fmaxf(nodeHierarchy[parentNode.childAIndex].aabb.by, nodeHierarchy[parentNode.childBIndex].aabb.by);
							parentNode.aabb.bz = fmaxf(nodeHierarchy[parentNode.childAIndex].aabb.bz, nodeHierarchy[parentNode.childBIndex].aabb.bz);
						}
						else
						{
							uncleIndex = grandParentNode.childBIndex;
							uncleNode = nodeHierarchy[uncleIndex];

							//Swap and update child and parent indices
							uncleNode.parentIndex = parentIndex;
							nodeToRotate.parentIndex = grandParentIndex;
							grandParentNode.childBIndex = currentIndex;
							if (parentNode.childAIndex == currentIndex)
							{
								parentNode.childAIndex = uncleIndex;
							}
							else
							{
								parentNode.childBIndex = uncleIndex;
							}

							//Refit parent aabb
							parentNode.aabb.ax = fminf(nodeHierarchy[parentNode.childAIndex].aabb.ax, nodeHierarchy[parentNode.childBIndex].aabb.ax);
							parentNode.aabb.ay = fminf(nodeHierarchy[parentNode.childAIndex].aabb.ay, nodeHierarchy[parentNode.childBIndex].aabb.ay);
							parentNode.aabb.az = fminf(nodeHierarchy[parentNode.childAIndex].aabb.az, nodeHierarchy[parentNode.childBIndex].aabb.az);
							parentNode.aabb.bx = fmaxf(nodeHierarchy[parentNode.childAIndex].aabb.bx, nodeHierarchy[parentNode.childBIndex].aabb.bx);
							parentNode.aabb.by = fmaxf(nodeHierarchy[parentNode.childAIndex].aabb.by, nodeHierarchy[parentNode.childBIndex].aabb.by);
							parentNode.aabb.bz = fmaxf(nodeHierarchy[parentNode.childAIndex].aabb.bz, nodeHierarchy[parentNode.childBIndex].aabb.bz);
						}

						//Test if cost decreased
						float d[3] = { nodeHierarchy[parentIndex].aabb.bx - nodeHierarchy[parentIndex].aabb.ax,  nodeHierarchy[parentIndex].aabb.by - nodeHierarchy[parentIndex].aabb.ay,  nodeHierarchy[parentIndex].aabb.bz - nodeHierarchy[parentIndex].aabb.az };

						double startingCost = 2 * (d[0] * d[1] + d[1] * d[2] + d[2] * d[0]);

						float d1[3] = { parentNode.aabb.bx - parentNode.aabb.ax,  parentNode.aabb.by - parentNode.aabb.ay,  parentNode.aabb.bz - parentNode.aabb.az };

						if ((2 * (d1[0] * d1[1] + d1[1] * d1[2] + d1[2] * d1[0])) < startingCost)
						{
							nodeHierarchy[uncleIndex] = uncleNode;
							nodeHierarchy[parentIndex] = parentNode;
							nodeHierarchy[currentIndex] = nodeToRotate;
							nodeHierarchy[grandParentIndex] = grandParentNode;
						}
					}
				}
			}
			currentIndex = nodeHierarchy[currentIndex].parentIndex;
		}
	}
}

unsigned int Mesh::GetMaxStackSize()
{
	std::stack<unsigned int> stack;
	unsigned int maxSize = 0;

	stack.push(rootIndex);

	while (stack.size() > 0)
	{
		uint32_t currentIndex = stack.top();
		stack.pop();
		if (!nodeHierarchy[currentIndex].isLeaf)
		{
			stack.push(nodeHierarchy[currentIndex].childAIndex);
			stack.push(nodeHierarchy[currentIndex].childBIndex);
		}
		maxSize = __max(maxSize, (unsigned int)stack.size());
	}

	return maxSize;
}