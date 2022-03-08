#define CASCADES 4

RWStructuredBuffer<float> g_mins : register(u0);
RWStructuredBuffer<float> g_maxes : register(u1);
RWStructuredBuffer<float> g_splits : register(u2);

[numthreads(4, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    // Assuming global min/max is in index 0 respectively
    float near = g_maxes[0];
    float far = g_mins[0];
    float partition = DTid.x + 1;

    float split = near * pow(abs(far / near), (float)partition  / CASCADES);

    g_splits[DTid.x] = split;
}