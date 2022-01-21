struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

static const float3 VERTS[] =
{
    float3(-1.0f, -1.0f, 0.f),
    float3(-1.f, 1.f, 0.f),
    float3(1.f, -1.f, 0.f),
    
    float3(-1.f, 1.f, 0.f),
    float3(1.f, 1.f, 0.f),
    float3(1.f, -1.f, 0.f)
};

static const float2 UVS[] =
{
    float2(0.f, 1.f),
    float2(0.f, 0.f),
    float2(1.f, 1.f),
    
    float2(0.f, 0.f),
    float2(1.f, 0.f),
    float2(1.f, 1.f)
};

//VertexOutput main(VertexInput input)
VertexOutput main(uint v_ID : SV_VertexID)
{
    VertexOutput output = (VertexOutput) 0;
    
    output.position = float4(VERTS[v_ID], 1.f);
    output.uv = UVS[v_ID];
    
    return output;
}