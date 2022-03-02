struct VertexInput
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 world : WORLD;
    float2 uv : UV;
    float3 normal : NORMAL;
};

cbuffer PerFrameCB : register(b0)
{
    matrix g_view_mat;
    matrix g_proj_mat;
}

cbuffer PerDraw : register(b1)
{
    matrix g_world_mat; 
}

VertexOutput main(VertexInput input)
//VertexOutput main(uint v_ID : SV_VertexID)
{
    VertexOutput output = (VertexOutput) 0;
    
    float4 world = mul(g_world_mat, float4(input.position, 1.f));
    output.world = world.rgb;
    output.position = mul(g_proj_mat, mul(g_view_mat, world));
    output.uv = input.uv;
    output.normal = input.normal;
    
    //output.position = float4(VERTS[v_ID], 1.f);
    //output.uv = 1.f.xx;
    //output.normal = COLORS[v_ID];
    
    return output;
}