#ifndef SHADERINTEROP_RENDERER_H
#define SHADERINTEROP_RENDERER_H
#include "ShaderInterop_Common.h"

// Cascaded Shadow Maps (Directional Light)
#define NUM_CASCADES 4
struct CascadeInfo
{
	float4x4 view_proj_mat;
	float near_z;
	float far_z;
};


// Deferred GBuffer Texture Slots
#define GBUFFER_ALBEDO_TEXTURE_SLOT 0
#define GBUFFER_NORMAL_TEXTURE_SLOT 1
#define GBUFFER_WORLD_TEXTURE_SLOT 2

// Per frame data that should be accessible at any stage during a frame
#define GLOBAL_PER_FRAME_CB_SLOT 13
struct PerFrameData
{
	float4x4 view_mat;
	float4x4 proj_mat;
	float4x4 inv_proj_mat;
};







#endif