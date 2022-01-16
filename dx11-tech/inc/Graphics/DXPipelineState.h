#pragma once
#include "DXDevice.h"
#include "GfxCommon.h"

struct DXPipelineState
{
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//ShaderHandle shader_program = { 0 };	// 0 should be an invalid shader for defaults
	InputLayoutPtr input_layout;

	RasterizerState1Ptr rasterizer;
	DepthStencilStatePtr depth_stencil;
	BlendState1Ptr blend;

};