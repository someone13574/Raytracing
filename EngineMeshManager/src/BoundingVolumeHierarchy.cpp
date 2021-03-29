#include "BoundingVolumeHierarchy.h"

#include <stack>

BoundingVolumeHierarchy::BoundingVolumeHierarchy() : mesh("Default Mesh")
{

}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(Mesh mesh) : mesh(mesh)
{
	ReadMesh(mesh);
}

void BoundingVolumeHierarchy::ReadMesh(Mesh mesh)
{
	nodeHierarchy.clear();

	for (uint32_t i = 0; i < (uint32_t)mesh.triangles.size(); i++)
	{
		uint32_t treeSize = (uint32_t)nodeHierarchy.size();

		Node node;
		node.triangleIndex = i;
		node.aabb = TriangleToAABB(mesh.uncompressedTriangles[i]);
		node.isLeaf = 1;
		node.parentIndex = 0;
		node.childAIndex = 4294967295;
		node.childBIndex = 4294967295;

		//Allocate Leaf Node
		uint32_t nodeIndex = AllocateNode(node);

		//Set to root node if necessary
		if (treeSize == 0)
		{
			rootIndex = nodeIndex;
			nodeHierarchy[nodeIndex].parentIndex = 4294967295;
			continue;
		}

		//Find best pair
		uint32_t sibling = rootIndex;
		std::stack<uint32_t> priorityQueue;
		double bestCost = INFINITY;
		priorityQueue.push(rootIndex);
		while (priorityQueue.size() > 0)
		{
			Node currentNode = nodeHierarchy[priorityQueue.top()];
			uint32_t currentNodeIndex = priorityQueue.top();
			priorityQueue.pop();

			//Direct cost
			AABB joined = JoinAABB(node.aabb, currentNode.aabb);
			double directCost = SurfaceArea(joined);

			//Indirect cost
			double indirectCost = 0;
			AABB refitted = joined;
			uint32_t currentIndex = currentNode.parentIndex;
			while (currentIndex != 4294967295)
			{
				refitted = JoinAABB(refitted, nodeHierarchy[currentIndex].aabb);
				indirectCost += SurfaceArea(refitted) - SurfaceArea(nodeHierarchy[currentIndex].aabb);
				currentIndex = nodeHierarchy[currentIndex].parentIndex;
			}

			//Update and add to queue
			double totalCost = directCost + indirectCost;
			if (totalCost < bestCost)
			{
				bestCost = totalCost;
				sibling = currentNodeIndex;

				if (currentNode.childAIndex != 4294967295)
				{
					double clow = SurfaceArea(node.aabb);

					refitted = currentNode.aabb;
					currentIndex = currentNode.parentIndex;
					while (currentIndex != 4294967295)
					{
						refitted = JoinAABB(refitted, nodeHierarchy[currentIndex].aabb);
						clow += SurfaceArea(refitted) - SurfaceArea(nodeHierarchy[currentIndex].aabb);
						currentIndex = nodeHierarchy[currentIndex].parentIndex;
					}

					//test if clow is lower then best cost
					if (clow < bestCost)
					{
						priorityQueue.push(currentNode.childAIndex);
						priorityQueue.push(currentNode.childBIndex);
					}
				}
			}
		}

		//Create parent
		uint32_t oldParent = nodeHierarchy[sibling].parentIndex;
		uint32_t newParent = AllocateNode({ 0, oldParent, 0, 0, JoinAABB(node.aabb, nodeHierarchy[sibling].aabb), 0 });

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
		uint32_t currentIndex = nodeHierarchy[nodeIndex].parentIndex;
		while (currentIndex != 4294967295)
		{
			nodeHierarchy[currentIndex].aabb = JoinAABB(nodeHierarchy[nodeHierarchy[currentIndex].childAIndex].aabb, nodeHierarchy[nodeHierarchy[currentIndex].childBIndex].aabb);
			Rotate(currentIndex);
			currentIndex = nodeHierarchy[currentIndex].parentIndex;
		}
	}

	tree.rootIndex = rootIndex;
	tree.nodeCount = (uint32_t)nodeHierarchy.size();
	tree.triangleCount = (uint32_t)mesh.uncompressedTriangles.size();
	tree.triangles = mesh.uncompressedTriangles;
	tree.nodeHierarchy = nodeHierarchy;
}

BoundingVolumeHierarchy::Tree BoundingVolumeHierarchy::GetTreeData() const
{
	return tree;
}

uint32_t BoundingVolumeHierarchy::AllocateNode(Node node)
{
	nodeHierarchy.push_back(node);
	return (uint32_t)nodeHierarchy.size() - 1;
}

BoundingVolumeHierarchy::AABB BoundingVolumeHierarchy::TriangleToAABB(Mesh::UncompressedTriangle triangle)
{
	AABB aabb;
	aabb.ax = fminf(triangle.vertices[0].position[0], fminf(triangle.vertices[1].position[0], triangle.vertices[2].position[0]));
	aabb.ay = fminf(triangle.vertices[0].position[1], fminf(triangle.vertices[1].position[1], triangle.vertices[2].position[1]));
	aabb.az = fminf(triangle.vertices[0].position[2], fminf(triangle.vertices[1].position[2], triangle.vertices[2].position[2]));

	aabb.bx = fmaxf(triangle.vertices[0].position[0], fmaxf(triangle.vertices[1].position[0], triangle.vertices[2].position[0]));
	aabb.by = fmaxf(triangle.vertices[0].position[1], fmaxf(triangle.vertices[1].position[1], triangle.vertices[2].position[1]));
	aabb.bz = fmaxf(triangle.vertices[0].position[2], fmaxf(triangle.vertices[1].position[2], triangle.vertices[2].position[2]));

	return aabb;
}

BoundingVolumeHierarchy::AABB BoundingVolumeHierarchy::JoinAABB(AABB a, AABB b)
{
	AABB aabb;
	aabb.ax = fminf(a.ax, b.ax);
	aabb.ay = fminf(a.ay, b.ay);
	aabb.az = fminf(a.az, b.az);

	aabb.bx = fmaxf(a.bx, b.bx);
	aabb.by = fmaxf(a.by, b.by);
	aabb.bz = fmaxf(a.bz, b.bz);

	return aabb;
}

unsigned int BoundingVolumeHierarchy::GetMaxStackSize()
{
	std::stack<uint32_t> stack;
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

float BoundingVolumeHierarchy::SurfaceArea(AABB aabb)
{
	float d[3] = { aabb.bx - aabb.ax,  aabb.by - aabb.ay,  aabb.bz - aabb.az };
	return 2 * (d[0] * d[1] + d[1] * d[2] + d[2] * d[0]);
}

void BoundingVolumeHierarchy::ReadMeshOptimized(Mesh mesh)
{
	nodeHierarchy.clear();

	for (uint32_t i = 0; i < (uint32_t)mesh.triangles.size(); i++)
	{
		uint32_t treeSize = (uint32_t)nodeHierarchy.size();

		Node node;
		node.triangleIndex = i;
		node.aabb = TriangleToAABB(mesh.uncompressedTriangles[i]);
		node.isLeaf = 1;
		node.parentIndex = 0;
		node.childAIndex = 4294967295;
		node.childBIndex = 4294967295;

		//Allocate Leaf Node
		uint32_t nodeIndex = AllocateNode(node);

		//Set to root node if necessary
		if (treeSize == 0)
		{
			rootIndex = nodeIndex;
			nodeHierarchy[nodeIndex].parentIndex = 4294967295;
			continue;
		}

		//Find best pair
		uint32_t sibling = rootIndex;
		std::stack<uint32_t> priorityQueue;
		double bestCost = INFINITY;
		priorityQueue.push(rootIndex);
		while (priorityQueue.size() > 0)
		{
			Node currentNode = nodeHierarchy[priorityQueue.top()];
			uint32_t currentNodeIndex = priorityQueue.top();
			priorityQueue.pop();

			//Direct cost
			AABB joined = JoinAABB(node.aabb, currentNode.aabb);
			double directCost = SurfaceArea(joined);

			//Indirect cost
			double indirectCost = 0;
			AABB refitted = joined;
			uint32_t currentIndex = currentNode.parentIndex;
			while (currentIndex != 4294967295)
			{
				refitted = JoinAABB(refitted, nodeHierarchy[currentIndex].aabb);
				indirectCost += SurfaceArea(refitted) - SurfaceArea(nodeHierarchy[currentIndex].aabb);
				currentIndex = nodeHierarchy[currentIndex].parentIndex;
			}

			//Update and add to queue
			double totalCost = directCost + indirectCost;
			if (totalCost < bestCost)
			{
				bestCost = totalCost;
				sibling = currentNodeIndex;

				if (currentNode.childAIndex != 4294967295)
				{
					double clow = SurfaceArea(node.aabb);

					refitted = currentNode.aabb;
					currentIndex = currentNode.parentIndex;
					while (currentIndex != 4294967295)
					{
						refitted = JoinAABB(refitted, nodeHierarchy[currentIndex].aabb);
						clow += SurfaceArea(refitted) - SurfaceArea(nodeHierarchy[currentIndex].aabb);
						currentIndex = nodeHierarchy[currentIndex].parentIndex;
					}

					//test if clow is lower then best cost
					if (clow < bestCost)
					{
						priorityQueue.push(currentNode.childAIndex);
						priorityQueue.push(currentNode.childBIndex);
					}
				}
			}
		}

		//Create parent
		uint32_t oldParent = nodeHierarchy[sibling].parentIndex;
		uint32_t newParent = AllocateNode({ 0, oldParent, 0, 0, JoinAABB(node.aabb, nodeHierarchy[sibling].aabb), 0 });

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
		uint32_t currentIndex = nodeHierarchy[nodeIndex].parentIndex;
		while (currentIndex != 4294967295)
		{
			nodeHierarchy[currentIndex].aabb = JoinAABB(nodeHierarchy[nodeHierarchy[currentIndex].childAIndex].aabb, nodeHierarchy[nodeHierarchy[currentIndex].childBIndex].aabb);
			Rotate(currentIndex);
			currentIndex = nodeHierarchy[currentIndex].parentIndex;
		}
	}

	tree.rootIndex = rootIndex;
	tree.nodeCount = (uint32_t)nodeHierarchy.size();
	tree.triangleCount = (uint32_t)mesh.triangles.size();
	tree.triangles = mesh.uncompressedTriangles;
	tree.nodeHierarchy = nodeHierarchy;
}

void BoundingVolumeHierarchy::Rotate(unsigned int index)
{
	Node node = nodeHierarchy[index];

	//Test if node has parent
	if (node.parentIndex != 4294967295)
	{
		unsigned int parentIndex = node.parentIndex;
		Node parentNode = nodeHierarchy[parentIndex];

		//Test if node has grand parent
		if (parentNode.parentIndex != 4294967295)
		{
			unsigned int grandParentIndex = parentNode.parentIndex;
			Node grandParentNode = nodeHierarchy[grandParentIndex];

			//Find uncle node
			unsigned int uncleIndex;
			Node uncleNode;
			if (grandParentNode.childAIndex != node.parentIndex)
			{
				uncleIndex = grandParentNode.childAIndex;
				uncleNode = nodeHierarchy[uncleIndex];

				//Swap and update child and parent indices
				uncleNode.parentIndex = parentIndex;
				node.parentIndex = grandParentIndex;
				grandParentNode.childAIndex = index;
				if (parentNode.childAIndex == index)
				{
					parentNode.childAIndex = uncleIndex;
				}
				else
				{
					parentNode.childBIndex = uncleIndex;
				}

				//Refit parent aabb
				parentNode.aabb = JoinAABB(nodeHierarchy[parentNode.childAIndex].aabb, nodeHierarchy[parentNode.childBIndex].aabb);
			}
			else
			{
				uncleIndex = grandParentNode.childBIndex;
				uncleNode = nodeHierarchy[uncleIndex];

				//Swap and update child and parent indices
				uncleNode.parentIndex = parentIndex;
				node.parentIndex = grandParentIndex;
				grandParentNode.childBIndex = index;
				if (parentNode.childAIndex == index)
				{
					parentNode.childAIndex = uncleIndex;
				}
				else
				{
					parentNode.childBIndex = uncleIndex;
				}

				//Refit parent aabb
				parentNode.aabb = JoinAABB(nodeHierarchy[parentNode.childAIndex].aabb, nodeHierarchy[parentNode.childBIndex].aabb);
			}

			//Test if cost decreased
			double startingCost = SurfaceArea(nodeHierarchy[parentIndex].aabb);
			if (SurfaceArea(parentNode.aabb) < startingCost)
			{
				nodeHierarchy[uncleIndex] = uncleNode;
				nodeHierarchy[parentIndex] = parentNode;
				nodeHierarchy[index] = node;
				nodeHierarchy[grandParentIndex] = grandParentNode;
			}
		}
	}
}

float BoundingVolumeHierarchy::ComputeTreeCost()
{
	float cost = 0.0f;
	for (unsigned int i = 0; i < tree.nodeCount; i++)
	{
		if (tree.nodeHierarchy[i].isLeaf == 0)
		{
			float d[3] = { tree.nodeHierarchy[i].aabb.bx - tree.nodeHierarchy[i].aabb.ax,  tree.nodeHierarchy[i].aabb.by - tree.nodeHierarchy[i].aabb.ay,  tree.nodeHierarchy[i].aabb.bz - tree.nodeHierarchy[i].aabb.az };
			cost += 2 * (d[0] * d[1] + d[1] * d[2] + d[2] * d[0]);
		}
	}

	return cost;
}