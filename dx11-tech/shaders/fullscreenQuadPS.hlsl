#include "TonemappingAlgorithms.hlsli"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

static const float GAMMA = 2.2f;

SamplerState lin_samp : register(s0);
Texture2D tex_pass : register(t0);

float4 main(PixelInput input) : SV_TARGET
{
    float3 hdr_col = tex_pass.Sample(lin_samp, input.uv).rgb;
    
    /*
        
        Do tonemapping before Gamma correction!
        Gamma correction should always be done last
        https://computergraphics.stackexchange.com/questions/5449/tone-mapping-gamma-correction/5451
        
        Real-Time Rendering 3rd by Möller, Haines and Hoffman, p. 145:
    */
    
    //for (int i = 0; i < 5000; ++i)
    //{
    //    hdr_col += i;
    //}
    
    float3 ldr_col = aces_fitted(hdr_col);
    //float3 ldr_col = hdr_col;

 
    // Gamma correction
    ldr_col = pow(abs(ldr_col), (1.f / GAMMA).xxx);
    
    return float4(ldr_col, 1.f);
}