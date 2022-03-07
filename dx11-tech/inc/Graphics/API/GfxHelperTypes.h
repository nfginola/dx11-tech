#pragma once
#include "Graphics/API/GfxCommon.h"
#include "DepthDefines.h"


// User can get an annotator from the GfxDevice
class GPUAnnotator
{
public:
	GPUAnnotator(AnnotationPtr annotator) : m_annotation(annotator) {}

	void begin_event(const std::string& name);
	void end_event();
	void set_marker(const std::string& name);

private:
	GPUAnnotator() = delete;
	GPUAnnotator& operator=(const GPUAnnotator&) = delete;
	GPUAnnotator(const GPUAnnotator&) = default;

private:
	AnnotationPtr m_annotation;
};


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

#ifdef REVERSE_Z_DEPTH
	DepthStencilClear() { *this = d0_s0(); }
#else
	DepthStencilClear() { *this = d1_s0(); }
#endif
	DepthStencilClear(FLOAT depth, UINT8 stencil, UINT clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL) :
		m_clear_flags(clear_flags),
		m_depth(depth),
		m_stencil(stencil)
	{}
	static DepthStencilClear d1_s0() { return DepthStencilClear(1.0f, 0); }
	static DepthStencilClear d0_s0() { return DepthStencilClear(0.0f, 0); }	

private:
	UINT m_clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL;

#ifdef REVERSE_Z_DEPTH
	FLOAT m_depth = 0.0f;
#else
	FLOAT m_depth = 1.f;
#endif

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

	// https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_subresource_data
	// Pitch and Slice pitch only meaningful for 2D and 3D texture data respectively
	SubresourceData(void* data, UINT pitch = 0, UINT slice_pitch = 0) : m_subres({ data, pitch, slice_pitch }) {}
private:
	D3D11_SUBRESOURCE_DATA m_subres{ nullptr, 0, 0 };
};

struct ShaderBytecode
{
	shared_ptr<std::vector<uint8_t>> code;
	std::string fname;
	// filepath -- std::string -- either .hlsl or .cso
	// we can use the extension to check if the pipeline is reloadable
};


class CopyRegionDst
{
	friend class GfxDevice;
public:
	CopyRegionDst() = delete;
	~CopyRegionDst() = default;
	CopyRegionDst& operator=(const CopyRegionDst&) = delete;
	CopyRegionDst(const CopyRegionDst&) = delete;

	CopyRegionDst(UINT subres_idx, UINT x = 0, UINT y = 0, UINT z = 0) : 
		m_subres(subres_idx), 
		m_x(x),
		m_y(y),
		m_z(z) {}

private:
	UINT m_subres = 0;
	UINT m_x = 0;
	UINT m_y = 0;
	UINT m_z = 0;
};

class CopyRegionSrc
{
	friend class GfxDevice;
public:
	CopyRegionSrc() = delete;
	~CopyRegionSrc() = default;
	CopyRegionSrc& operator=(const CopyRegionDst&) = delete;
	CopyRegionSrc(const CopyRegionDst&) = delete;
	CopyRegionSrc(UINT subres_idx, const D3D11_BOX& box, UINT copy_flags = D3D11_COPY_DISCARD) :
		m_subres(subres_idx),
		m_box(box),
		m_copy_flags(copy_flags) {}

private:
	UINT m_subres = 0;
	D3D11_BOX m_box = {};
	UINT m_copy_flags;
};
