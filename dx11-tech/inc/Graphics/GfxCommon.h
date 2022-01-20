#pragma once
#include <optional>
#include <variant>
#include "Graphics/DXDevice.h"

namespace gfxconstants
{
	static constexpr UINT MAX_CS_UAV = D3D11_PS_CS_UAV_REGISTER_COUNT;
	static constexpr UINT MAX_SHADER_INPUT_RESOURCE_SLOTS = D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT;
	static constexpr UINT MAX_SCISSORS = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	static constexpr UINT MAX_VIEWPORTS = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	static constexpr UINT MAX_RENDER_TARGETS = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
	static constexpr FLOAT MIN_DEPTH = D3D11_MIN_DEPTH;
	static constexpr FLOAT MAX_DEPTH = D3D11_MAX_DEPTH;
}

enum class ShaderStage { eNone, eVertex, eHull, eDomain, eGeometry, ePixel, eCompute};
enum class BufferType { eNone, eConstant, eVertex, eIndex, eStructured, eAppendConsume, eByteAddress, eRaw, eCustom };
enum class TextureType { eNone, e1D, e2D, e3D };
enum class DepthFormat { eD32, eD32_S8, eD24_S8 };





