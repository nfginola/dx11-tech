#pragma once
#include <stdint.h>

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
