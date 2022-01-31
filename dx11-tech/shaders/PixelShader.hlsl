struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

float4 main(PixelInput input) : SV_TARGET
{
    float3 col = normalize(input.normal);
    
    //return float4(0.f, 1.f, 0.f, 1.f);
    
    return float4(col, 1.f);
    //return g_vec;
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
}