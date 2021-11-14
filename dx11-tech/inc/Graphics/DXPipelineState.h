#pragma once
#include "DXDevice.h"
#include "GfxCommon.h"

struct DXPipelineState
{
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	InputLayoutPtr input_layout;
	
	std::array<D3D11_VIEWPORT, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE> viewports_and_scissors;
	std::array<D3D11_RECT, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE> scissors;
	RasterizerState1Ptr rasterizer;

	DepthStencilStatePtr depth_stencil;
	BlendStatePtr blend;

	ShaderHandle shader_program = { 0 };	// 0 should be an invalid shader for defaults
};