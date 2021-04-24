cbuffer RadixSortConstants : register(b1, space0)
{
	unsigned int shift;
};

RWStructuredBuffer<uint2> mortonValueBuffer : register(u7);
RWStructuredBuffer<unsigned int> histogramPrefixSumBuffer : register(u8);
RWStructuredBuffer<unsigned int> tempHistogramPrefixSumBuffer : register(u9);

[numthreads(1024, 1, 1)]
void main(uint id : SV_DispatchThreadID)
{
	uint2 mortonValues[CHUNK_SIZE]; //Copy of this chunks morton codes final rearrangement

	bool chunkValues[CHUNK_SIZE];
	unsigned int localPrefixSum[CHUNK_SIZE];

	unsigned int histogram[2] = { 0, 0 };

	for (unsigned int i = 0; i < CHUNK_SIZE; i++) //Extract bits from morton code, calculate local prefix sum, and build local histogram
	{
		mortonValues[i] = mortonValueBuffer[id * CHUNK_SIZE + i];
		if (shift < 32)
		{
			chunkValues[i] = ((mortonValues[i].y >> shift) & 1);
		}
		else
		{
			chunkValues[i] = ((mortonValues[i].x >> abs(shift - 32)) & 1);
		}
		localPrefixSum[i] = histogram[chunkValues[i]];
		histogram[chunkValues[i]]++;
	}

	histogramPrefixSumBuffer[id] = histogram[0];
	histogramPrefixSumBuffer[id + 1024] = histogram[1];

	unsigned int threadIndexA = id * 2; //Index this thread is responsable for in calculating global prefix sum
	unsigned int threadIndexB = threadIndexA + 1;

	AllMemoryBarrierWithGroupSync();
	tempHistogramPrefixSumBuffer[threadIndexA] = histogramPrefixSumBuffer[threadIndexA - 1] + histogramPrefixSumBuffer[threadIndexA];
	tempHistogramPrefixSumBuffer[threadIndexB] = histogramPrefixSumBuffer[threadIndexB - 1] + histogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	histogramPrefixSumBuffer[threadIndexA] = tempHistogramPrefixSumBuffer[threadIndexA - 2] + tempHistogramPrefixSumBuffer[threadIndexA];
	histogramPrefixSumBuffer[threadIndexB] = tempHistogramPrefixSumBuffer[threadIndexB - 2] + tempHistogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	tempHistogramPrefixSumBuffer[threadIndexA] = histogramPrefixSumBuffer[threadIndexA - 4] + histogramPrefixSumBuffer[threadIndexA];
	tempHistogramPrefixSumBuffer[threadIndexB] = histogramPrefixSumBuffer[threadIndexB - 4] + histogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	histogramPrefixSumBuffer[threadIndexA] = tempHistogramPrefixSumBuffer[threadIndexA - 8] + tempHistogramPrefixSumBuffer[threadIndexA];
	histogramPrefixSumBuffer[threadIndexB] = tempHistogramPrefixSumBuffer[threadIndexB - 8] + tempHistogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	tempHistogramPrefixSumBuffer[threadIndexA] = histogramPrefixSumBuffer[threadIndexA - 16] + histogramPrefixSumBuffer[threadIndexA];
	tempHistogramPrefixSumBuffer[threadIndexB] = histogramPrefixSumBuffer[threadIndexB - 16] + histogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	histogramPrefixSumBuffer[threadIndexA] = tempHistogramPrefixSumBuffer[threadIndexA - 32] + tempHistogramPrefixSumBuffer[threadIndexA];
	histogramPrefixSumBuffer[threadIndexB] = tempHistogramPrefixSumBuffer[threadIndexB - 32] + tempHistogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	tempHistogramPrefixSumBuffer[threadIndexA] = histogramPrefixSumBuffer[threadIndexA - 64] + histogramPrefixSumBuffer[threadIndexA];
	tempHistogramPrefixSumBuffer[threadIndexB] = histogramPrefixSumBuffer[threadIndexB - 64] + histogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	histogramPrefixSumBuffer[threadIndexA] = tempHistogramPrefixSumBuffer[threadIndexA - 128] + tempHistogramPrefixSumBuffer[threadIndexA];
	histogramPrefixSumBuffer[threadIndexB] = tempHistogramPrefixSumBuffer[threadIndexB - 128] + tempHistogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	tempHistogramPrefixSumBuffer[threadIndexA] = histogramPrefixSumBuffer[threadIndexA - 256] + histogramPrefixSumBuffer[threadIndexA];
	tempHistogramPrefixSumBuffer[threadIndexB] = histogramPrefixSumBuffer[threadIndexB - 256] + histogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	histogramPrefixSumBuffer[threadIndexA] = tempHistogramPrefixSumBuffer[threadIndexA - 512] + tempHistogramPrefixSumBuffer[threadIndexA];
	histogramPrefixSumBuffer[threadIndexB] = tempHistogramPrefixSumBuffer[threadIndexB - 512] + tempHistogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	tempHistogramPrefixSumBuffer[threadIndexA] = histogramPrefixSumBuffer[threadIndexA - 1024] + histogramPrefixSumBuffer[threadIndexA];
	tempHistogramPrefixSumBuffer[threadIndexB] = histogramPrefixSumBuffer[threadIndexB - 1024] + histogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	histogramPrefixSumBuffer[threadIndexA] = tempHistogramPrefixSumBuffer[threadIndexA - 2048] + tempHistogramPrefixSumBuffer[threadIndexA];
	histogramPrefixSumBuffer[threadIndexB] = tempHistogramPrefixSumBuffer[threadIndexB - 2048] + tempHistogramPrefixSumBuffer[threadIndexB];

	AllMemoryBarrierWithGroupSync();
	tempHistogramPrefixSumBuffer[threadIndexA] = histogramPrefixSumBuffer[threadIndexA - 1];
	tempHistogramPrefixSumBuffer[threadIndexB] = histogramPrefixSumBuffer[threadIndexB - 1];

	AllMemoryBarrierWithGroupSync();

	for (unsigned int i = 0; i < CHUNK_SIZE; i++) //Get final positions
	{
		mortonValueBuffer[tempHistogramPrefixSumBuffer[id + 1024 * chunkValues[i]] + localPrefixSum[i]] = mortonValues[i];
	}
}