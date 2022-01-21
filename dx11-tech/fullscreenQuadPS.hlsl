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
    float3 col = tex_pass.Sample(lin_samp, input.uv).rgb;
    
    // Gamma correction
    col = pow(abs(col), (1.f / GAMMA).xxx);
    
    return float4(col, 1.f);
}