#include "ShaderInterop_Renderer.h"

struct VertexInput
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 world : WORLD;
    float2 uv : UV;
    float3 normal : NORMAL;
};

CBUFFER(PerFrameCB, GLOBAL_PER_FRAME_CB_SLOT)
{
    PerFrameData g_per_frame;
}

cbuffer PerDraw : register(b1)
{
    matrix g_world_mat; 
}

VertexOutput main(VertexInput input)
{
    VertexOutput output = (VertexOutput) 0;
    
    float4 world = mul(g_world_mat, float4(input.position, 1.f));
    output.world = world.rgb;
    output.position = mul(g_per_frame.proj_mat, mul(g_per_frame.view_mat, world));
    output.uv = input.uv;
    output.normal = input.normal;
    
    return output;
}