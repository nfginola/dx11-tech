#include "ShaderInterop_Renderer.h"
#include "../inc/DepthDefines.h"            // Reverse Z-depth on/off

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

CBUFFER(PerFrameCB, GLOBAL_PER_FRAME_CB_SLOT)
{
    PerFrameData g_per_frame;
}

CBUFFER(PerLightCB, 1)
{
    float4 g_light_direction;
}

READ_RESOURCE(Texture2D, g_albedo, GBUFFER_ALBEDO_TEXTURE_SLOT)
READ_RESOURCE(Texture2D, g_normal, GBUFFER_NORMAL_TEXTURE_SLOT)
READ_RESOURCE(Texture2D, g_world, GBUFFER_WORLD_TEXTURE_SLOT)

READ_RESOURCE(Texture2D, g_main_depth, 6)
READ_RESOURCE(Texture2DArray, g_directional_cascades_sm, 7)
READ_RESOURCE(StructuredBuffer<CascadeInfo>, g_directional_cascades_info, 8)

SAMPLER(lin_samp, 0)
SAMPLER(sm_samp, 3)

float4 main(PixelInput input) : SV_TARGET0
{
    float3 col = g_albedo.Sample(lin_samp, input.uv).rgb;
    
    float3 nor = g_normal.Sample(lin_samp, input.uv).rgb;
    float3 world = g_world.Sample(lin_samp, input.uv).rgb;

    float3 ambient = col * 0.1f;

    float curr_pixel_view_z = mul(g_per_frame.view_mat, float4(world, 1.f)).z;
    //float4 curr_pixel_clip = mul(g_per_frame.proj_mat, mul(g_per_frame.view_mat, float4(world, 1.f)));
    //float curr_pixel_ndc_z = (curr_pixel_clip / curr_pixel_clip.w).z;


    float is_shadowed = 0.f;    // Lit
    float3 tint = 0.f.xxx;
    [unroll]
    for (int cascade = 0; cascade < NUM_CASCADES; ++cascade)
    {   
        //float4 p = float4(0.f, 0.f, g_directional_cascades_info[cascade].far_z, 1.f);
        //p = mul(g_per_frame.proj_mat, p);
        //p /= p.w;

        if (curr_pixel_view_z < g_directional_cascades_info[cascade].far_z)
        //if (curr_pixel_ndc_z > p.z)
        {
            if (cascade == 0)
                tint = float3(1.f, 0.f, 0.f) * 0.1f;
            else if (cascade == 1)
                tint = float3(0.f, 1.f, 0.f) * 0.1f;
            else if (cascade == 2)
                tint = float3(0.f, 0.f, 1.f) * 0.1f;
            else
                tint = float3(1.f, 0.f, 1.f) * 0.1f;

            float4 lspace_clip = mul(g_directional_cascades_info[cascade].view_proj_mat, float4(world, 1.f));
            float4 lspace_ndc = lspace_clip / lspace_clip.w; // persp dvision

            // ndc has 0,0 mid and 1,1 top right --> need to convert to texture space: 0,0 top left, 1,1 bottom left
            float2 lspace_uv = float2(lspace_ndc.x * 0.5f + 0.5f, 1.f - (lspace_ndc.y * 0.5f + 0.5f));

            float ldepth = g_directional_cascades_sm.Sample(sm_samp, float3(lspace_uv, cascade)).r; // Depth of this pixel from shadow persp.
            float real_depth = lspace_ndc.z; // Depth of curr frag 

            float bias = max(0.001 * (1.0 - normalize(dot(nor, g_light_direction.xyz))), 0.0005);
            //float bias = 0.f;
            //float bias = max(0.0001 * (1.0 - normalize(dot(nor, g_light_direction.xyz))), 0.00001);

#ifdef REVERSE_Z_DEPTH
            if (real_depth + bias < ldepth)
#else
    if (real_depth - bias > ldepth)
#endif
            {
                is_shadowed = 1.f; // Not lit
            }
            break;
        }
    }

            
 
    float dir_light_contrib = max(dot(nor, -g_light_direction.xyz), 0.f) * 1.f; // intensity
    float3 diffuse = col * dir_light_contrib;       // if you want to add tint, multiplicative blend is the way (represent light factor absorbed per channel)


    float3 final_color = ambient + diffuse * (1.f - is_shadowed);
    final_color += tint;

    return float4(final_color, 1.f);
}