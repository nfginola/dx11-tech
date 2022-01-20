#pragma once
#include "GfxCommon.h"

class RenderTextureClear
{
	friend class GfxDevice;
public:
	RenderTextureClear(std::array<float, 4> rgba = { 0.f, 0.f, 0.f, 1.f }) :
		m_rgba(rgba)
	{}

	static RenderTextureClear black() { return RenderTextureClear(); }

private:
	std::array<float, 4> m_rgba = { 0.f, 0.f, 0.f, 1.f };

};

class DepthStencilClear
{
	friend class GfxDevice;
public:
	DepthStencilClear(FLOAT depth, UINT8 stencil, UINT clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL) :
		m_clear_flags(clear_flags),
		m_depth(depth),
		m_stencil(stencil)
	{}
	static DepthStencilClear d1_s0() { return DepthStencilClear(1.0f, 0); }

private:
	UINT m_clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL;
	FLOAT m_depth = 1.0f;
	UINT8 m_stencil = 0;
};

class ReadWriteClear
{
	friend class GfxDevice;
public:
	ReadWriteClear() = delete;

	// Will trigger ClearUnorderedAccessViewFloat
	static ReadWriteClear fp(FLOAT a, FLOAT b, FLOAT c, FLOAT d) { return ReadWriteClear(std::array<FLOAT, 4>{a, b, c, d}); }

	// Will trigger ClearUnorderedAccessViewUint
	static ReadWriteClear uint(UINT a, UINT b, UINT c, UINT d) { return ReadWriteClear(std::array<UINT, 4>{a, b, c, d}); }

private:
	ReadWriteClear(std::variant<std::array<UINT, 4>, std::array<FLOAT, 4>> clear) : m_clear(clear) {};

private:
	std::variant<std::array<UINT, 4>, std::array<FLOAT, 4>> m_clear;
};

class SubresourceData
{
	friend class GfxDevice;
public:
	SubresourceData() = default;
	SubresourceData(void* data, UINT pitch, UINT slice_pitch) : m_subres({ data, pitch, slice_pitch }) {}
private:
	D3D11_SUBRESOURCE_DATA m_subres{ nullptr, 0, 0 };
};

struct ShaderBytecode
{
	shared_ptr<std::vector<uint8_t>> code;
};



