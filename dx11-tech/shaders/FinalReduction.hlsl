RWStructuredBuffer<float> g_input_values : register(u0);
RWStructuredBuffer<float> g_mins : register(u1);

#define BLOCK_DIM 1024

groupshared float z_values[BLOCK_DIM];
groupshared float z_mins[BLOCK_DIM];

[numthreads(BLOCK_DIM, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint Tidx : SV_GroupIndex, uint3 Gid : SV_GroupID)
{    
    // Place into shared memory first
    z_values[Tidx] = g_input_values[DTid.x];

    if (DTid.x >= 2040)
        z_mins[Tidx] = 1.f;
    else
        z_mins[Tidx] = g_mins[DTid.x];

    AllMemoryBarrierWithGroupSync();

    // Do step-wise halving to find min/max
    for (int s = 1; s < BLOCK_DIM; s *= 2)
    {
        if (Tidx % (2 * s) == 0)
        {
            z_values[Tidx] = max(z_values[Tidx], z_values[Tidx + s]);

            if ((Tidx + s) < BLOCK_DIM)
                z_mins[Tidx] = min(z_mins[Tidx], z_mins[Tidx + s]);
            
        }
        AllMemoryBarrierWithGroupSync();
    }
    
    if (Tidx == 0)
    {
        g_input_values[Gid.x] = z_values[0];
        g_mins[Gid.x] = z_mins[0];

    }
}