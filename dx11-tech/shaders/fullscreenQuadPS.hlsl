struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

static const float GAMMA = 2.2f;

SamplerState lin_samp : register(s0);
Texture2D g_albedo : register(t0);
Texture2D g_normal : register(t1);
Texture2D g_world : register(t2);

float4 main(PixelInput input) : SV_TARGET0
{
    float3 col = g_albedo.Sample(lin_samp, input.uv).rgb;

    return float4(col, 1.f);
}