#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <assert.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

using DeviceChildPtr = ComPtr<ID3D11DeviceChild>;

using SwapChainPtr = ComPtr<IDXGISwapChain>;
using DevicePtr = ComPtr<ID3D11Device>;
using DeviceContextPtr = ComPtr<ID3D11DeviceContext>;
using Device1Ptr = ComPtr<ID3D11Device1>;					// 11_1
using DeviceContext1Ptr = ComPtr<ID3D11DeviceContext1>;		// 11_1
using DebugPtr = ComPtr<ID3D11Debug>;
using FactoryPtr = ComPtr<IDXGIFactory>;

// Query
using QueryPtr = ComPtr<ID3D11Query>;

// Async 
using AsyncPtr = ComPtr<ID3D11Asynchronous>;

// Event annotation
using AnnotationPtr = ComPtr<ID3DUserDefinedAnnotation>;


// Common resources
using GPUResourcePtr = ComPtr<ID3D11Resource>;
using Tex1DPtr = ComPtr<ID3D11Texture1D>;
using Tex2DPtr = ComPtr<ID3D11Texture2D>;
using Tex3DPtr = ComPtr<ID3D11Texture3D>;
using BufferPtr = ComPtr<ID3D11Buffer>;
using BlendStatePtr = ComPtr<ID3D11BlendState>;
using BlendState1Ptr = ComPtr<ID3D11BlendState1>;
using SamplerStatePtr = ComPtr<ID3D11SamplerState>;
using DepthStencilStatePtr = ComPtr<ID3D11DepthStencilState>;
using InputLayoutPtr = ComPtr<ID3D11InputLayout>;
using RasterizerStatePtr = ComPtr<ID3D11RasterizerState>;
using RasterizerState1Ptr = ComPtr<ID3D11RasterizerState1>;

// Views
using ViewPtr = ComPtr<ID3D11View>;
using SrvPtr = ComPtr<ID3D11ShaderResourceView>;
using RtvPtr = ComPtr<ID3D11RenderTargetView>;
using UavPtr = ComPtr<ID3D11UnorderedAccessView>;
using DsvPtr = ComPtr<ID3D11DepthStencilView>;
using AdapterPtr = ComPtr<IDXGIAdapter>;

// Shaders
using VsPtr = ComPtr<ID3D11VertexShader>;
using HsPtr = ComPtr<ID3D11HullShader>;
using DsPtr = ComPtr<ID3D11DomainShader>;
using GsPtr = ComPtr<ID3D11GeometryShader>;
using PsPtr = ComPtr<ID3D11PixelShader>;
using CsPtr = ComPtr<ID3D11ComputeShader>;

using BlobPtr = ComPtr<ID3DBlob>;



bool HRCHECK(HRESULT hr);

class DXDevice
{


public:
	DXDevice(HWND hwnd, int bbWidth, int bbHeight);
	~DXDevice();

	DXDevice& operator=(const DXDevice&) = delete;
	DXDevice(const DXDevice&) = delete;

	//void Recreate_swapchain(int newWidth, int newHeight);
	//DevicePtr get_device();
	//DeviceContextPtr get_context();

	HWND get_hwnd() const;
	const Device1Ptr& get_device() const;
	const DeviceContext1Ptr& get_context() const;
	
	const SwapChainPtr& get_sc() const;
	const DXGI_SWAP_CHAIN_DESC& get_sc_desc() const;
	
	const Tex2DPtr& get_bb_texture() const;
	const RtvPtr& get_bb_target() const;
	const D3D11_VIEWPORT& get_bb_viewport() const;

	const AnnotationPtr get_annotation() const;

private:
	void create_device_and_context();
	void get_debug();
	void get_display_modes();
	void create_swapchain(HWND hwnd, int bbWidth, int bbHeight);
	void create_bb_target();

private:
	HWND m_hwnd;
	DevicePtr m_device;
	DeviceContextPtr m_context;
	Device1Ptr m_device_1;			// 11_1
	DeviceContext1Ptr m_context_1;	// 11_1
	AdapterPtr m_adapter;
	FactoryPtr m_factory;
	DebugPtr m_debug;

	SwapChainPtr m_sc;
	DXGI_SWAP_CHAIN_DESC m_sc_desc;
	Tex2DPtr m_bbTex;
	RtvPtr m_bbView;
	D3D11_VIEWPORT m_bbViewport;

	AnnotationPtr m_annotation;

	std::vector<DXGI_MODE_DESC> m_available_display_modes;
};


