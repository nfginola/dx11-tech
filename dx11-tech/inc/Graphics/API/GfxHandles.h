#pragma once
#include "NamedType.h"
#include <stdint.h>

#define USE_64_BIT_RES_HANDLE

#ifdef USE_64_BIT_RES_HANDLE
using res_handle = uint64_t;
#else
using res_handle = uint32_t;
#endif


struct PipelineHandle { res_handle hdl = 0; };
struct BufferHandle { res_handle hdl = 0; };
struct TextureHandle { res_handle hdl = 0; };
struct SamplerHandle { res_handle hdl = 0; };
struct ShaderHandle { res_handle hdl = 0; };
struct RenderPassHandle { res_handle hdl = 0; };

// Strongly typed shaders for safer public interface
// https://www.fluentcpp.com/2016/12/08/strong-types-for-strong-interfaces/
using VertexShader = NamedType<ShaderHandle, struct VertexShaderPhantom>;
using PixelShader = NamedType<ShaderHandle, struct PixelShaderPhantom>;
using GeometryShader = NamedType<ShaderHandle, struct GeometryShaderPhantom>;
using HullShader = NamedType<ShaderHandle, struct HullShaderPhantom>;
using DomainShader = NamedType<ShaderHandle, struct DomainShaderPhantom>;
using ComputeShader = NamedType<ShaderHandle, struct ComputeShaderPhantom>;

bool operator==(const PipelineHandle& lhs, const PipelineHandle& rhs);
bool operator!=(const PipelineHandle& lhs, const PipelineHandle& rhs);
bool operator<(const PipelineHandle& lhs, const PipelineHandle& rhs);
bool operator>(const PipelineHandle& lhs, const PipelineHandle& rhs);

bool operator==(const BufferHandle& lhs, const BufferHandle& rhs);
bool operator!=(const BufferHandle& lhs, const BufferHandle& rhs);
bool operator<(const BufferHandle& lhs, const BufferHandle& rhs);
bool operator>(const BufferHandle& lhs, const BufferHandle& rhs);

bool operator==(const TextureHandle& lhs, const TextureHandle& rhs);
bool operator!=(const TextureHandle& lhs, const TextureHandle& rhs);
bool operator<(const TextureHandle& lhs, const TextureHandle& rhs);
bool operator>(const TextureHandle& lhs, const TextureHandle& rhs);

bool operator==(const SamplerHandle& lhs, const SamplerHandle& rhs);
bool operator!=(const SamplerHandle& lhs, const SamplerHandle& rhs);
bool operator<(const SamplerHandle& lhs, const SamplerHandle& rhs);
bool operator>(const SamplerHandle& lhs, const SamplerHandle& rhs);

bool operator==(const ShaderHandle& lhs, const ShaderHandle& rhs);
bool operator!=(const ShaderHandle& lhs, const ShaderHandle& rhs);
bool operator<(const ShaderHandle& lhs, const ShaderHandle& rhs);
bool operator>(const ShaderHandle& lhs, const ShaderHandle& rhs);

bool operator==(const RenderPassHandle& lhs, const RenderPassHandle& rhs);
bool operator!=(const RenderPassHandle& lhs, const RenderPassHandle& rhs);
bool operator<(const RenderPassHandle& lhs, const RenderPassHandle& rhs);
bool operator>(const RenderPassHandle& lhs, const RenderPassHandle& rhs);

namespace std {

    template <>
    struct hash<TextureHandle>
    {
        std::size_t operator()(const TextureHandle& k) const
        {
            return std::hash<res_handle>{}(k.hdl);
        }
    };
}
