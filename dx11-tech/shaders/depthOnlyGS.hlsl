#include "ShaderInterop_Renderer.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
    uint rt_idx : SV_RenderTargetArrayIndex;
};

READ_RESOURCE(StructuredBuffer<CascadeInfo>, CascadeInfoSB, 0)

[maxvertexcount(3)]
[instance(NUM_CASCADES)]
void main(
	triangle float4 input[3] : SV_POSITION,
	uint instance_id : SV_GSInstanceID,
	inout TriangleStream< GSOutput > output
)
{

	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
        float4 clip_space = mul(CascadeInfoSB[instance_id].view_proj_mat, input[i]);
		element.pos = clip_space;
        element.rt_idx = instance_id;
		output.Append(element);
	}
}