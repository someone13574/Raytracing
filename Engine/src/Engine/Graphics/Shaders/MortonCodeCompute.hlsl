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

StructuredBuffer<Triangle> triangleBuffer : register(t1);
RWStructuredBuffer<Vertex> vertexBuffer : register(u2);
RWStructuredBuffer<uint2> mortonValueBuffer : register(u7);

uint2 ExpandBits(unsigned int temp1)
{
	temp1 &= 0x001fffff;
	unsigned int temp2 = temp1;
	temp1 &= 0x0000ffff;
	temp2 &= 0xf8000000;

	temp2 |= temp1 >> 16;
	temp1 |= temp1 << 16;
	temp1 &= 0xff0000ff;
	temp2 &= 0x001f0000;

	temp2 |= temp1 >> 24;
	temp1 |= temp1 << 8;
	temp1 &= 0x0f00f00f;
	temp2 &= 0x100f00f0;

	temp2 |= temp1 >> 28;
	temp1 |= temp1 << 4;
	temp1 &= 0xc30c30c3;
	temp2 &= 0x10c30c30;

	temp2 |= temp1 >> 30;
	temp1 |= temp1 << 2;
	temp1 &= 0x49249249;
	temp2 &= 0x12492492;

	return uint2(0x00000000, temp1 & 0x0000000F); //uint2(temp2, temp1);
}

uint2 GetMortonCode(float3 position)
{
	position *= 0.1;
	position += 0.5;

	uint3 f = min(max(position * 2097152.0f, 0), 2097151.0f);

	uint2 ex = ExpandBits(f.x);
	uint2 ey = ExpandBits(f.y);
	uint2 ez = ExpandBits(f.z);

	return uint2((ex.y >> 30) | (ey.y >> 31) | (ex.x << 2) | (ey.x << 1) | ez.x, (ex.y << 2) | (ey.y << 1) | ez.y);
}

[numthreads(1024, 1, 1)]
void main (uint id : SV_DispatchThreadID)
{
	Vertex vertex1 = vertexBuffer[(triangleBuffer[id].indices1 & 0x000fffff)];
	Vertex vertex2 = vertexBuffer[((triangleBuffer[id].indices1 & 0xfff00000) >> 20) | ((triangleBuffer[id].indices2 & 0x00000ff) << 12)];
	Vertex vertex3 = vertexBuffer[((triangleBuffer[id].indices2 & 0x0fffff00) >> 8)];

	float3 center = 0.33333 * (float3(vertex1.position) + float3(vertex2.position) + float3(vertex3.position));

	mortonValueBuffer[id] = GetMortonCode(center);
}