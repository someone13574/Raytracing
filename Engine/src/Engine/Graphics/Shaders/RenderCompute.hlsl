#define mod(x, y) (x - y * floor(x / y))

struct Triangle
{
	float normal[3];
	float vertexPositions[3][3];
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
	unsigned int width;
	unsigned int height;

	float originX;
	float originY;
	float originZ;

	float x0, y0, z0, x1;
	float y1, z1, x2, y2;
	float z2;
}

float3 triIntersect(float3 rayOrigin, float3 rayDirection, float vertexPositions[3][3])
{
	float3 v1v0 = float3(vertexPositions[1][0], vertexPositions[1][1], vertexPositions[1][2]) - float3(vertexPositions[0][0], vertexPositions[0][1], vertexPositions[0][2]);
	float3 v2v0 = float3(vertexPositions[2][0], vertexPositions[2][1], vertexPositions[2][2]) - float3(vertexPositions[0][0], vertexPositions[0][1], vertexPositions[0][2]);
	float3 ov0 = rayOrigin - float3(vertexPositions[0][0], vertexPositions[0][1], vertexPositions[0][2]);
	float3 n = cross(v1v0, v2v0);
	float3 q = cross(ov0, rayDirection);
	float d = 1.0 / dot(rayDirection, n);
	float u = d * dot(-q, v2v0);
	float v = d * dot(q, v1v0);
	float t = d * dot(-n, ov0);

	if (u < 0.0 || u > 1.0 || v < 0.0 || (u + v) > 1.0 || t < 0)
	{
		t = 1.#INF;
	}

	return float3(t, u, v);
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

bool boxIntersection(float3 ro, float3 frd, float3 minc, float3 maxc)
{
	float tx1 = (minc.x - ro.x) * frd.x;
	float tx2 = (maxc.x - ro.x) * frd.x;

	float tmin = min(tx1, tx2);
	float tmax = max(tx1, tx2);

	float ty1 = (minc.y - ro.y) * frd.y;
	float ty2 = (maxc.y - ro.y) * frd.y;

	tmin = max(tmin, min(ty1, ty2));
	tmax = min(tmax, max(ty1, ty2));

	float tz1 = (minc.z - ro.z) * frd.z;
	float tz2 = (maxc.z - ro.z) * frd.z;

	tmin = max(tmin, min(tz1, tz2));
	tmax = min(tmax, max(tz1, tz2));

	return tmax >= tmin;
}

RayHit SampleScene(float3 rayOrigin, float3 rayDirection)
{
	RayHit hit;
	hit.color = float3(0.5, 0.5, 0.5);
	hit.distance = 1.#INF;
	hit.position = float3(0, 0, 0);
	hit.normal = float3(0, 1, 0);

	uint stack[MAX_STACK_SIZE];
	uint stackIndex = 1;
	stack[0] = ROOT_NODE_INDEX;

	float3 fractionalRayDirection = 1 / rayDirection;
	while (stackIndex > 0)
	{
		BVHNode node = nodeHierarchy[stack[stackIndex - 1]];
		if (boxIntersection(rayOrigin, fractionalRayDirection, float3(node.ax, node.ay, node.az), float3(node.bx, node.by, node.bz)))
		{
			stackIndex--;
			if (node.isLeaf == 0)
			{
				stack[stackIndex] = node.childAIndex;
				stack[stackIndex + 1] = node.childBIndex;
				stackIndex += 2;
			}
			else
			{
				float3 dst = triIntersect(rayOrigin, rayDirection, triangleBuffer[node.triangleIndex].vertexPositions);
				if (dst.x < hit.distance)
				{
					hit.distance = dst.x;
					hit.normal = (float3)triangleBuffer[node.triangleIndex].normal;
					hit.color = hit.normal;
				}
			}
		}
		else
		{
			stackIndex--;
		}
	}

	//ground
	float t = -rayOrigin.y / rayDirection.y;
	if (t > 0 && t < hit.distance && length(rayOrigin + rayDirection * t) < 400)
	{
		hit.distance = t;
		hit.position = rayOrigin + rayDirection * t;
		hit.color = saturate(filteredChecker(hit.position.xz * 0.01) + 0.5);
	}
	//ground

	hit.position = (hit.distance * rayDirection + rayOrigin) + hit.normal;

	return hit;
}

[numthreads(32, 32, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
	float2 uv = (-float2(width, height) + 2 * id) / height;

	float3 rayOrigin = float3(originX, originY, originZ);

	float3 rayDirection = normalize(mul(float3(uv.x, -uv.y, -1.3), float3x3(x0, y0, z0, x1, y1, z1, x2, y2, z2)));

	const float3 lightDir = float3(0.577350269, 0.577350269, 0.577350269);

	RayHit hit = SampleScene(rayOrigin, rayDirection);

	float diffuse = saturate(dot(hit.normal, lightDir));
	float lighting = saturate(diffuse + 0.05);
	result[id] = float4(hit.color, 1);
}