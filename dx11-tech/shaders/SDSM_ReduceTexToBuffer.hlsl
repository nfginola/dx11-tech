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
CBUFFER(PerFrameCB, PER_FRAME_CB_SLOT)
{
    PerFrameData g_per_frame;
}


groupshared float z_values[BLOCK_DIM];
groupshared float z_mins[BLOCK_DIM];


// https://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
float3 view_space_pos(uint2 pixel, float depth)
{
    float2 uv = float2(pixel.x / 1920.f, pixel.y / 1080.f);

    uv = saturate(uv);

    float x = uv.x * 2.f - 1.f;
    float y = (1.f - uv.y) * 2.f - 1.f;
    float4 ndc = float4(x, y, depth, 1.f);

    // Unproject
    float4 vs_pos = mul(g_per_frame.inv_proj_mat, ndc);
    return (vs_pos.xyz / vs_pos.w).xyz;

}

[numthreads(BLOCK_DIM_X, BLOCK_DIM_Y, 1)]
void main( uint3 DTid : SV_DispatchThreadID, uint Tidx : SV_GroupIndex, uint3 Gid : SV_GroupID)
{
    // Place into shared memory first
    //float val = g_test_texture[DTid.xy];
    float val = g_depth[DTid.xy];
    //float val = view_space_pos(DTid.xy, g_depth[DTid.xy]).z;
    //if (DTid.x > 1920 || DTid.y > 1080)
    //    val = 1.f;

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

    if (Tidx == 0)
    {
        g_output_buf[Gid.x + Gid.y * 60] = z_values[0];
        g_output_buf2[Gid.x + Gid.y * 60] = z_mins[0];
    }
}