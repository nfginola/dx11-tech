#pragma once
#include "Graphics/GfxCommon.h"

class GPUType
{
public:
	bool is_empty() const { return m_internal_resource == nullptr; }

protected:
	GPUType() = default;		// Not a public object!

	DeviceChildPtr m_internal_resource;
};

class GPUResource : public GPUType
{
	friend class GfxApi;
protected:
	SrvPtr m_srv;
	UavPtr m_uav;
	RtvPtr m_rtv;
};

class GPUTexture : public GPUResource
{
	friend class GfxApi;
private:
	TextureType m_type = TextureType::eNone;
	DsvPtr m_dsv;
};

class GPUBuffer : public GPUResource
{
	friend class GfxApi;
private:
	BufferType m_type = BufferType::eNone;
};

class Shader : public GPUType
{
	friend class GraphicsPipeline;
	friend class GfxApi;
private:
	ShaderStage m_stage = ShaderStage::eNone;
};

class Sampler : public GPUType { friend class GfxApi; };

class RasterizerState : public GPUType { friend class GfxApi; };

class InputLayout : public GPUType { friend class GfxApi; };

class BlendState : public GPUType { friend class GfxApi; };

class DepthStencilState : public GPUType { friend class GfxApi; };

class Framebuffer {};

class GraphicsPipeline {};

class ComputePipeline {};

class RenderPass {};
