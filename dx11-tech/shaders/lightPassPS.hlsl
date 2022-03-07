struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

cbuffer PerFrameData : register(b0)
{
    matrix g_view_mat;
    matrix g_proj_mat;
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

    //float bias = max(0.005 * (1.0 - normalize(dot(nor, g_light_direction.xyz))), 0.0012);
    float bias = max(0.0001 * (1.0 - normalize(dot(nor, g_light_direction.xyz))), 0.00001);


    float shadow_factor = 1.f;      // Lit

    //if (real_depth - bias > ldepth)
    if (real_depth + bias < ldepth)
    {
        shadow_factor = 0.f; // Not lit
    }

            
 
    float dir_light_contrib = max(dot(nor, -g_light_direction.xyz), 0.f) * 1.f; // intensity
    float3 diffuse = col * dir_light_contrib;       // if you want to add tint, multiplicative blend is the way (represent light factor absorbed per channel)


    return float4(ambient + diffuse * shadow_factor, 1.f);
}