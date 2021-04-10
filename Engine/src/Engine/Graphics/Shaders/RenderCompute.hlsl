#pragma warning( disable : 4000 )

struct Triangle
{
	uint indices1;
	uint indices2;
};
struct Vertex
{
	float position[3];
	float normal[3];
	float UV[2];
};

struct BVHNode
{
	uint triangleIndex;

	uint parentIndex;
	uint childAIndex;
	uint childBIndex;

	float ax;
	float ay;
	float az;
	float bx;
	float by;
	float bz;
	int isLeaf;
};
struct RayHit
{
	float distance;
	float3 normal;
	float3 position;
	float3 color;
};

RWTexture2D<float4> result : register(u0);
RWStructuredBuffer<BVHNode> nodeHierarchy : register(u1);
RWStructuredBuffer<Vertex> vertexBuffer : register(u2);
RWTexture2D<float4> tempResult : register(u3);
RWTexture2D<float4> reprojectionBuffer : register(u4);

struct UIElement
{
	uint xPosition;
	uint yPosition;
	uint xScale;
	uint yScale;

	float roundness;

	float r;
	float g;
	float b;
};

StructuredBuffer<UIElement> uiElements : register(t0);
StructuredBuffer<Triangle> triangleBuffer : register(t1);

cbuffer constants : register(b0, space0)
{
	float time;
	unsigned int frame;
	unsigned int width;
	unsigned int height;

	float originX;
	float originY;
	float originZ;

	float previousOriginX;
	float previousOriginY;
	float previousOriginZ;

	float2 padding;

	float4 mat0;
	float4 mat1;
	float4 mat2;

	float4 previousMat0;
	float4 previousMat1;
	float4 previousMat2;
}

float4 triIntersect(float3 origin, float3 rayDirection, float3 v0, float3 v1, float3 v2)
{
	float3 v1v0 = v1 - v0;
	float3 v2v0 = v2 - v0;
	float3 n = cross(v1v0, v2v0);
	float d = dot(rayDirection, n);
	if (abs(d) < 0.0000001)
	{
		return float4(1.#INF, 0, 0, 0);
	}
	d = 1.0 / d;
	float3 q = cross(origin - v0, rayDirection);
	float u = d * dot(-q, v2v0);
	if (u < 0 || u > 1)
	{
		return float4(1.#INF, 0, 0, 0);
	}
	float v = d * dot(q, v1v0);
	if (v < 0 || u + v > 1)
	{
		return float4(1.#INF, 0, 0, 0);
	}
	float t = d * dot(-n, origin - v0);
	if (t < 0)
	{
		return float4(1.#INF, 0, 0, 0);
	}

	return float4(t, u, v, 1 - u - v);
}

float4 sampleUI(float2 position)
{
	float minDst = 1.#INF;

	uint count = 0;
	uint stride = 0;
	uiElements.GetDimensions(count, stride);

	uint colorId = 0;
	for (uint i = 0; i < count; i++)
	{
		UIElement element = uiElements[i];
		float2 d = abs(float2(element.xPosition, element.yPosition) - position) - (float2(element.xScale, element.yScale) - element.roundness);
		float dst = length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - element.roundness;
		if (dst < minDst)
		{
			minDst = dst;
			colorId = i;
		}
	}

	return (count != 0) ? float4(uiElements[colorId].r, uiElements[colorId].g, uiElements[colorId].b, minDst) : 1;
}

float filteredChecker(float2 p)
{
	float2 s = sign(frac(p*.5) - .5);
	return .5 - .5*s.x*s.y;
}

bool boxIntersection(float3 origin, float3 inverseDirection, float3 lowerCorner, float3 upperCorner)
{
	float3 t1 = (lowerCorner - origin) * inverseDirection;
	float3 t2 = (upperCorner - origin) * inverseDirection;

	return min(min(max(t1.x, t2.x), max(t1.y, t2.y)), max(t1.z, t2.z)) >= max(max(min(t1.x, t2.x), min(t1.y, t2.y)), min(t1.z, t2.z));
}

RayHit SampleScene(float3 rayOrigin, float3 rayDirection)
{
	RayHit hit;
	hit.color = float3(0.5, 0.5, 0.5);
	hit.distance = 1.#INF;
	hit.position = float3(0, 0, 0);
	hit.normal = float3(0, 1, 0);

	unsigned int currentIndex = ROOT_NODE_INDEX;
	unsigned int lastIndex = 4294967295;

	float3 fractionalRayDirection = 1 / rayDirection;
	while (currentIndex != 4294967295u)
	{
		BVHNode node = nodeHierarchy[currentIndex];

		if (lastIndex == node.parentIndex)
		{
			if (boxIntersection(rayOrigin, fractionalRayDirection, float3(node.ax, node.ay, node.az), float3(node.bx, node.by, node.bz)))
			{
				if (node.isLeaf == 0)
				{
					lastIndex = currentIndex;
					currentIndex = node.childAIndex;
					continue;
				}
				else
				{
					uint index1 = (triangleBuffer[node.triangleIndex].indices1 & 0x000fffff);
					uint index2 = ((triangleBuffer[node.triangleIndex].indices1 & 0xfff00000) >> 20) | ((triangleBuffer[node.triangleIndex].indices2 & 0x00000ff) << 12);
					uint index3 = ((triangleBuffer[node.triangleIndex].indices2 & 0x0fffff00) >> 8);

					float4 intersect = triIntersect(rayOrigin, rayDirection, (float3)vertexBuffer[index1].position, (float3)vertexBuffer[index2].position, (float3)vertexBuffer[index3].position);
					if (intersect.x < hit.distance)
					{
						hit.distance = intersect.x;
						hit.normal = normalize(((float3)vertexBuffer[index2].normal * intersect.y) + ((float3)vertexBuffer[index3].normal * intersect.z) + ((float3)vertexBuffer[index1].normal * intersect.w));

						float2 uv = (float2)vertexBuffer[index2].UV * intersect.y + (float2)vertexBuffer[index3].UV * intersect.z + (float2)vertexBuffer[index1].UV * intersect.w;

						hit.color = float3(0.2, 0.8, 1);
					}
				}
			}
			lastIndex = currentIndex;
			currentIndex = node.parentIndex;
			continue;
		}
		else
		{
			bool tempbool = lastIndex == node.childAIndex;
			lastIndex = currentIndex;
			currentIndex = (tempbool) ? node.childBIndex : node.parentIndex;
		}
	}

	float t = -rayOrigin.y / rayDirection.y;
	if (t > 0 && t < hit.distance && length(rayOrigin + rayDirection * t) < 4)
	{
		hit.distance = t;
		hit.position = rayOrigin + rayDirection * t;
		hit.color = saturate(filteredChecker(hit.position.xz * 2) + 0.5);
	}

	hit.position = (hit.distance * rayDirection + rayOrigin);

	return hit;
}

float hash(uint state)
{
	state ^= 2747636419u;
	state *= 2654435769u;
	state ^= state >> 16;
	state *= 2654435769u;
	state ^= state >> 16;
	state *= 2654435769u;
	return (float)state * 0.0000000002328306437;
}

float4 sampleTempBuffer(float2 pos)
{
	float2 fpos = pos - floor(pos);

	float4 col1 = tempResult[uint2(pos.x, pos.y)] * (1 - fpos.x) * (1 - fpos.y);
	float4 col2 = tempResult[uint2(pos.x, pos.y + 1)] * (1 - fpos.x) * fpos.y;
	float4 col3 = tempResult[uint2(pos.x + 1, pos.y)] * fpos.x * (1 - fpos.y);
	float4 col4 = tempResult[uint2(pos.x + 1, pos.y + 1)] * fpos.x * fpos.y;

	return col1 + col2 + col3 + col4;
}

[numthreads(32, 32, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
	float2 offset = float2(hash(frac(time) * 100), hash(frac(time) * 100 + 1));

	float2 uv = (-int2(width, height) + 2.0 * (id + offset)) / (float)height;

	float3 rayOrigin = float3(originX, originY, originZ);

	float3 rayDirection = normalize(mul(float3(uv.x, -uv.y, -1.5), float3x3(mat0.x, mat0.y, mat0.z, mat0.w, mat1.x, mat1.y, mat1.z, mat1.w, mat2.x)));

	float4 color = float4(0, 0, 0, 0);
	RayHit hit = SampleScene(rayOrigin, rayDirection);
	if (hit.distance != 1.#INF)
	{
		float random = hash(id.x * width + id.y + (frac(time + 1) * 65536));
		float3 lightDir = float3(0.57735 + random * 0.05, 0.57735 + hash(random * 65536 + 1) * 0.05, 0.57735 + hash(random * 65536) * 0.05);
		bool inShadow = SampleScene(hit.position + (hit.normal * 0.001), lightDir).distance == 1.#INF;
		float lighting = (saturate(dot(hit.normal, float3(0.57735, 0.57735, 0.57735))) * inShadow) + 0.05;

		color = float4(hit.color * lighting, 1);
	}
	else
	{
		hit.distance = -1;
		color = float4(hit.color, 1);
	}

	//Reproject
	float3 cameraSpace = mul(float3x3(previousMat0.x, previousMat0.y, previousMat0.z, previousMat0.w, previousMat1.x, previousMat1.y, previousMat1.z, previousMat1.w, previousMat2.x), (hit.distance * rayDirection) + rayOrigin - float3(previousOriginX, previousOriginY, previousOriginZ));
	float2 ndc = -1.5 * cameraSpace.xy / cameraSpace.z;

	float2 rasterSpace = ((float2(ndc.x, -ndc.y) * (float)height + float2(width, height)) / 2.0) - offset;

	float4 reprojectedColor = sampleTempBuffer(rasterSpace);

	if (abs(hit.distance - reprojectedColor.w) > 0.1 || rasterSpace.x < 0 || rasterSpace.y < 0 || frame == 0)
	{
		reprojectionBuffer[id] = float4(color.xyz, hit.distance);
	}
	else
	{
		reprojectionBuffer[id] = float4(color.xyz * 0.01 + reprojectedColor.xyz * 0.99, hit.distance);
	}

	result[id] = reprojectionBuffer[id];
}