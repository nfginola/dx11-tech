#include "ShaderInterop_Common.h"

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 world : WORLD;
    float2 uv : UV;
    float3 normal : NORMAL;
};

struct PixelOutput
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 world : SV_TARGET2;
};

READ_RESOURCE(Texture2D, main_tex, 0)
SAMPLER(repeat_samp, 1)

PixelOutput main(PixelInput input)
{
    float3 col = normalize(input.normal);
    
    //return float4(0.f, 1.f, 0.f, 1.f);
    
    col = main_tex.Sample(repeat_samp, input.uv).rgb;
    //col = float3(input.uv, 0.f);
    
    PixelOutput output = (PixelOutput) 0;

    output.albedo = float4(col, 1.f);
    output.normal = float4(normalize(float3(input.normal)), 1.f);
    output.world = float4(input.world, 1.f);

    return output;
    //return g_vec;
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
}