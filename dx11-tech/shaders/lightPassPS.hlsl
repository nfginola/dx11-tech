#include "ShaderInterop_Renderer.h"
#include "../inc/DepthDefines.h"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

CBUFFER(PerFrameCB, PER_FRAME_CB_SLOT)
{
    PerFrameData g_per_frame;
}

cbuffer PerLightData : register(b1)
{
    matrix g_light_view_proj;
    matrix g_light_view_proj_inv;
    float4 g_light_direction;
}

Texture2D g_albedo : register(t0);
Texture2D g_normal : register(t1);
Texture2D g_world : register(t2);

//StructuredBuffer<float> g_splits : register(t5);
Texture2D g_main_depth : register(t6);
Texture2D g_directional_sm : register(t7);

SamplerState lin_samp : register(s0);
//SamplerState point_samp : register(s1);
SamplerState sm_samp : register(s3);


float4 main(PixelInput input) : SV_TARGET0
{
    float3 col = g_albedo.Sample(lin_samp, input.uv).rgb;
    
    float3 nor = g_normal.Sample(lin_samp, input.uv).rgb;
    float3 world = g_world.Sample(lin_samp, input.uv).rgb;

    float3 ambient = col * 0.1f;



    // try reconstruction some other time, use gbuffer worldspace to get into lightspace ndc

    float4 lspace_clip = mul(g_light_view_proj, float4(world, 1.f));
    float4 lspace_ndc = lspace_clip / lspace_clip.w;     // persp dvision
    
    // ndc has 0,0 mid and 1,1 top right --> need to convert to texture space: 0,0 top left, 1,1 bottom left
    float2 lspace_uv = float2(lspace_ndc.x * 0.5f + 0.5f, 1.f - (lspace_ndc.y * 0.5f + 0.5f));

    float ldepth = g_directional_sm.Sample(sm_samp, lspace_uv).r;       // Depth of this pixel from shadow persp.
    float real_depth = lspace_ndc.z;                                    // Depth of curr frag 

    float bias = max(0.005 * (1.0 - normalize(dot(nor, g_light_direction.xyz))), 0.0012);
    //float bias = max(0.0001 * (1.0 - normalize(dot(nor, g_light_direction.xyz))), 0.00001);


    float shadow_factor = 1.f;      // Lit

    #ifdef REVERSE_Z_DEPTH
    if (real_depth + bias < ldepth)
    #else
    if (real_depth - bias > ldepth)
    #endif
    {
        shadow_factor = 0.f; // Not lit
    }

            
 
    float dir_light_contrib = max(dot(nor, -g_light_direction.xyz), 0.f) * 1.f; // intensity
    float3 diffuse = col * dir_light_contrib;       // if you want to add tint, multiplicative blend is the way (represent light factor absorbed per channel)


    //float curr_view_depth = g_main_depth.Sample(sm_samp, input.uv).r;
    //float3 tint;
    //float tint_strength = 0.05;
    //if (curr_view_depth > g_splits[0])
    //{
    //    tint = float3(1.f, 0.f, 0.f) * tint_strength;
    //}
    //else if (curr_view_depth > g_splits[1])
    //{
    //    tint = float3(0.f, 1.f, 0.f) * tint_strength;
    //}
    //else if (curr_view_depth > g_splits[2])
    //{
    //    tint = float3(0.f, 0.f, 1.f) * tint_strength;
    //}
    //else
    //{
    //    tint = float3(0.0f, 1.f, 1.f) * tint_strength;
    //}

    float3 final_color = ambient + diffuse * shadow_factor;
    //final_color += tint;

    return float4(final_color, 1.f);
}