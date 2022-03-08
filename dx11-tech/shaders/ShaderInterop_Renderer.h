#ifndef SHADERINTEROP_RENDERER_H
#define SHADERINTEROP_RENDERER_H
#include "ShaderInterop_Common.h"


#define PER_FRAME_CB_SLOT 13
struct PerFrameData
{
	matrix view_mat;
	matrix proj_mat;
	matrix inv_proj_mat;
};





#endif