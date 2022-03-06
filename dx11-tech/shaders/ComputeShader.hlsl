RWTexture2D<float> g_test_texture : register(u0);
RWTexture2D<float> g_test_texture_output : register(u1);
RWStructuredBuffer<float> g_output_buf : register(u2);
RWStructuredBuffer<float> g_output_buf2 : register(u3);

Texture2D<float> g_depth : register(t0);

#define BLOCK_DIM_X 32
#define BLOCK_DIM_Y 32
#define BLOCK_DIM BLOCK_DIM_X * BLOCK_DIM_Y

groupshared float z_values[BLOCK_DIM];

groupshared float z_mins[BLOCK_DIM];

[numthreads(BLOCK_DIM_X, BLOCK_DIM_Y, 1)]
void main( uint3 DTid : SV_DispatchThreadID, uint Tidx : SV_GroupIndex, uint3 Gid : SV_GroupID)
{
    // Place into shared memory first
    //float val = g_test_texture[DTid.xy];
    float val = g_depth[DTid.xy];
    z_values[Tidx] = val;

    // Make sure to not accidentally grab 0s for min
    if (DTid.y >= 1080)         // Branch on the blocks in the last row
        z_mins[Tidx] = 1.f;
    else
        z_mins[Tidx] = val;

    AllMemoryBarrierWithGroupSync();
    
    // Do step-wise halving to find min/max
    for (int s = 1; s < BLOCK_DIM; s *= 2)
    {
        if (Tidx % (2 * s) == 0)
        {
            z_values[Tidx] = max(z_values[Tidx], z_values[Tidx + s]);
            z_mins[Tidx] = min(z_mins[Tidx], z_mins[Tidx + s]);
        }
        AllMemoryBarrierWithGroupSync();
    }
    
    //g_test_texture_output[DTid.xy] = z_values[0];

    if (Tidx == 0)
    {
        g_output_buf[Gid.x + Gid.y * 60] = z_values[0];
        g_output_buf2[Gid.x + Gid.y * 60] = z_mins[0];
    }
}