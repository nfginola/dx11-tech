#include "pch.h"
#include "Graphics/API/GfxHandles.h"

bool operator==(const PipelineHandle& lhs, const PipelineHandle& rhs) { return lhs.hdl == rhs.hdl; }
bool operator<(const PipelineHandle& lhs, const PipelineHandle& rhs) { return lhs.hdl < rhs.hdl; }
bool operator>(const PipelineHandle& lhs, const PipelineHandle& rhs) { return lhs.hdl > rhs.hdl; }

bool operator==(const BufferHandle& lhs, const BufferHandle& rhs) { return lhs.hdl == rhs.hdl; }
bool operator<(const BufferHandle& lhs, const BufferHandle& rhs) { return lhs.hdl < rhs.hdl; }
bool operator>(const BufferHandle& lhs, const BufferHandle& rhs) { return lhs.hdl > rhs.hdl; }

bool operator==(const TextureHandle& lhs, const TextureHandle& rhs) { return lhs.hdl == rhs.hdl; }
bool operator<(const TextureHandle& lhs, const TextureHandle& rhs) { return lhs.hdl < rhs.hdl; }
bool operator>(const TextureHandle& lhs, const TextureHandle& rhs) { return lhs.hdl > rhs.hdl; }

bool operator==(const SamplerHandle& lhs, const SamplerHandle& rhs) { return lhs.hdl == rhs.hdl; }
bool operator<(const SamplerHandle& lhs, const SamplerHandle& rhs) { return lhs.hdl < rhs.hdl; }
bool operator>(const SamplerHandle& lhs, const SamplerHandle& rhs) { return lhs.hdl > rhs.hdl; }

bool operator==(const ShaderHandle& lhs, const ShaderHandle& rhs) { return lhs.hdl == rhs.hdl; }
bool operator<(const ShaderHandle& lhs, const ShaderHandle& rhs) { return lhs.hdl < rhs.hdl; }
bool operator>(const ShaderHandle& lhs, const ShaderHandle& rhs) { return lhs.hdl > rhs.hdl; }

bool operator==(const RenderPassHandle& lhs, const RenderPassHandle& rhs) { return lhs.hdl == rhs.hdl; }
bool operator<(const RenderPassHandle& lhs, const RenderPassHandle& rhs) { return lhs.hdl < rhs.hdl; }
bool operator>(const RenderPassHandle& lhs, const RenderPassHandle& rhs) { return lhs.hdl > rhs.hdl; }