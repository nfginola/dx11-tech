#ifndef SHADERINTEROP_COMMON_H
#define SHADERINTEROP_COMMON_H

#ifdef __cplusplus

#include <DirectXMath.h>

/*
	Ensure that when we include any other ShaderInterop which have defined structs in terms of HLSL,
	we get a corresponding version on the CPP side
*/

using matrix = DirectX::XMMATRIX;
using float4x4 = DirectX::XMFLOAT4X4;
using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;
using uint = uint32_t;


#endif

#define CBUFFER(name, slot) cbuffer name : register(b##slot)						// Uses buffer slots
#define READ_RESOURCE(type, name, slot) type name : register(t##slot);				// Uses shader resource slots
#define READ_WRITE_RESOURCE(type, name, slot) type name : register(u##slot);		// Uses unordered access slots
#define SAMPLER(name, slot) SamplerState name : register(s##slot);					// Uses sampler slots


#endif