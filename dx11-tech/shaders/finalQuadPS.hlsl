#include "TonemappingAlgorithms.hlsli"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

static const float GAMMA = 2.2f;

SAMPLER(lin_samp, 0)
READ_RESOURCE(Texture2D, g_final_tex, 0)

float4 main(PixelInput input) : SV_TARGET
{
    /*
        Negative color produces weird values for the HDR to LDR conversion..
    */
    float3 hdr_col = max(g_final_tex.Sample(lin_samp, input.uv).rgb, 0.f.xxx);
    
    /*
        Do tonemapping before Gamma correction!
        Gamma correction should always be done last
        https://computergraphics.stackexchange.com/questions/5449/tone-mapping-gamma-correction/5451
        
        Real-Time Rendering 3rd by M?ller, Haines and Hoffman, p. 145:
    */
    
    //for (int i = 0; i < 5000; ++i)
    //{
    //    hdr_col += i;
    //}
    
    float3 ldr_col = aces_fitted(hdr_col);
    //float3 ldr_col = hdr_col;

    // Gamma correction
    ldr_col = pow(abs(ldr_col), (1.f / GAMMA).xxx);

    //float2 p = (2.0 * input.uv * float2(1920, 1080) - float2(1920, 1080)) / 1080;
    //ldr_col = smoothstep(2.0, 0.01, length(p)) * ldr_col;

    // luminosity
    //ldr_col = sqrt(0.299 * pow(ldr_col.r, 2) + 0.587 * pow(ldr_col.g, 2) + 0.114 * pow(ldr_col.b, 2)).xxx;

    return float4(ldr_col, 1.f);
}