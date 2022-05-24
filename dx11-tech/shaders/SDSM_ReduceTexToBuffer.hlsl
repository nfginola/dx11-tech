#include "ShaderInterop_Renderer.h"

Texture2D<float> g_depth : register(t0);
RWStructuredBuffer<float> g_output_buf : register(u0);
RWStructuredBuffer<float> g_output_buf2 : register(u1);


#define BLOCK_DIM_X 32
#define BLOCK_DIM_Y 32
#define BLOCK_DIM BLOCK_DIM_X * BLOCK_DIM_Y

//cbuffer PerFrameCB : register(b0)
//{
//    PerFrameData g_per_frame;
//}
CBUFFER(PerFrameCB, GLOBAL_PER_FRAME_CB_SLOT)
{
    PerFrameData g_per_frame;
}


groupshared float z_values[BLOCK_DIM];
groupshared float z_mins[BLOCK_DIM];

[numthreads(BLOCK_DIM_X, BLOCK_DIM_Y, 1)]
void main( uint3 DTid : SV_DispatchThreadID, uint Tidx : SV_GroupIndex, uint3 Gid : SV_GroupID)
{
;
    float val = g_depth[DTid.xy];

    // Place into shared memory first
    z_values[Tidx] = val;

    // Make sure to not accidentally grab 0s for min
    if (DTid.y >= 1080)         // Branch on the blocks in the last row
        z_mins[Tidx] = 1.f;
    else
        z_mins[Tidx] = val;

    AllMemoryBarrierWithGroupSync();
    
    if (z_mins[Tidx] < 0.00001f)
        z_mins[Tidx] = 1.f;
    
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

    if (Tidx == 0)
    {
        g_output_buf[Gid.x + Gid.y * 60] = z_values[0];
        g_output_buf2[Gid.x + Gid.y * 60] = z_mins[0];
    }
}