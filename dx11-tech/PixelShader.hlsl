struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

static const float GAMMA = 2.2f;

float4 main(PixelInput input) : SV_TARGET
{
    float3 col = input.normal;
    col = pow(abs(col), (1.f / GAMMA).xxx);
    
    return float4(col, 1.f);
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
}