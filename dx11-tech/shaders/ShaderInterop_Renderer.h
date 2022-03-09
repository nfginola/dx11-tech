#ifndef SHADERINTEROP_RENDERER_H
#define SHADERINTEROP_RENDERER_H
#include "ShaderInterop_Common.h"

// Cascaded Shadow Maps (Directional Light)
#define NUM_CASCADES 4

// Deferred GBuffer Texture Slots
#define GBUFFER_ALBEDO_TEXTURE_SLOT 0
#define GBUFFER_NORMAL_TEXTURE_SLOT 1
#define GBUFFER_WORLD_TEXTURE_SLOT 2

// Per frame data that should be accessible at any stage during a frame
#define GLOBAL_PER_FRAME_CB_SLOT 13
struct PerFrameData
{
	matrix view_mat;
	matrix proj_mat;
	matrix inv_proj_mat;
};





#endif