#include "pch.h"
#include "Graphics/API/DXDevice.h"

// Lets us print some debug messages sent to the queue
// Currently responsible for printing WARNING debug messages to cout
ComPtr<ID3D11InfoQueue> s_info_queue;

bool HRCHECK(HRESULT hr)
{
	bool passed = SUCCEEDED(hr);

	// We have set a BREAK on CORRUPTION and ERRORS which are the major validators, no need to print out anything
	// If we do print out anything to the console, it'll be the warnings :)
	if (!passed && s_info_queue != nullptr)
	{
		// https://stackoverflow.com/questions/53579283/directx-11-debug-layer-capture-error-strings
		HRESULT hr = s_info_queue->PushEmptyStorageFilter();
		assert(hr == S_OK);

		UINT64 msgCount = s_info_queue->GetNumStoredMessages();
		for (UINT64 i = 0; i < msgCount; i++) {

			// Enumerate
			SIZE_T msgSize = 0;
			s_info_queue->GetMessage(i, nullptr, &msgSize); 
			
			// Fill
			D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(msgSize); 
			hr = s_info_queue->GetMessage(i, message, &msgSize);
			assert(hr == S_OK);
			
			// Log
			std::cout << message->pDescription << '\n';

			free(message);
		}
		s_info_queue->ClearStoredMessages();
	}

	assert(passed);
	return passed;
}

DXDevice::DXDevice(HWND hwnd, int bbWidth, int bbHeight) :
	m_hwnd(hwnd)
{
	create_device_and_context();
	get_debug();
	get_display_modes();
	create_swapchain(hwnd, bbWidth, bbHeight);
	create_bb_target();

	// Grab annotation
	HRCHECK(m_context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)m_annotation.GetAddressOf()));

	// Prevent DXGI from responding to Mode Changes and Alt + Enter (We will handle this ourselves)
	//m_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);		// orig
	m_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);	
	//m_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES);
}

DXDevice::~DXDevice()
{
	s_info_queue.Reset();

	// Some things will still be alive.. notably members of this class since their ref count is decremented at some point during this destructor..
	//m_debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
}


void DXDevice::create_device_and_context()
{
	std::array<D3D_FEATURE_LEVEL, 2> featureLevels{ D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1 };		// 11.1
	UINT flags = D3D11_CREATE_DEVICE_DEBUG/*| D3D11_CREATE_DEVICE_SINGLETHREADED*/;		// We may have async resource creation

	HRCHECK(
		D3D11CreateDevice(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			flags,
			featureLevels.data(),
			static_cast<UINT>(featureLevels.size()),
			D3D11_SDK_VERSION,
			m_device.GetAddressOf(),
			NULL,
			m_context.GetAddressOf()
		)
	);

	// Get factory associated with the created device
	ComPtr<IDXGIDevice> dxgi_dev;
	HRCHECK(m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgi_dev.GetAddressOf()));
	HRCHECK(dxgi_dev->GetParent(__uuidof(IDXGIAdapter), (void**)m_adapter.GetAddressOf()));
	HRCHECK(m_adapter->GetParent(__uuidof(IDXGIFactory), (void**)m_factory.GetAddressOf()));

	// Get 11.1 Device and Context
	HRCHECK(m_device->QueryInterface(__uuidof(ID3D11Device1), (void**)m_device_1.GetAddressOf()));
	m_device_1->GetImmediateContext1(m_context_1.GetAddressOf());
}

void DXDevice::get_debug()
{
	HRCHECK(m_device->QueryInterface(__uuidof(ID3D11Debug), (void**)m_debug.GetAddressOf()));
	HRCHECK(m_debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)s_info_queue.GetAddressOf()));
	s_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	s_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
	s_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
	s_info_queue->SetMuteDebugOutput(false);
}

void DXDevice::get_display_modes()
{
	// DXGIOutput --> Represents an adapter output (e.g monitor)	
	ComPtr<IDXGIOutput> dxgi_out;
	HRCHECK(m_adapter->EnumOutputs(0, dxgi_out.GetAddressOf()));

	UINT num_modes = 0;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;	// Format to look for

	// Enumerate
	HRCHECK(dxgi_out->GetDisplayModeList(format, 0, &num_modes, NULL));

	// Fill list
	m_available_display_modes.resize(num_modes);
	HRCHECK(dxgi_out->GetDisplayModeList(format, 0, &num_modes, m_available_display_modes.data()));
}

void DXDevice::create_swapchain(HWND hwnd, int bbWidth, int bbHeight)
{
	DXGI_MODE_DESC bestMode = m_available_display_modes.back();

	m_sc_desc.BufferDesc.Width = bbWidth;
	m_sc_desc.BufferDesc.Height = bbHeight;
	m_sc_desc.BufferDesc.RefreshRate.Numerator = bestMode.RefreshRate.Numerator;
	m_sc_desc.BufferDesc.RefreshRate.Denominator = bestMode.RefreshRate.Denominator;
	//m_sc_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	m_sc_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_sc_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	m_sc_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// SC backbuffer is not a multisampled image
	m_sc_desc.SampleDesc.Quality = 0;
	m_sc_desc.SampleDesc.Count = 1;

	// Surface usage is back buffer!
	m_sc_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_sc_desc.BufferCount = s_buffer_count;		// One front and one back buffer

	m_sc_desc.OutputWindow = hwnd;
	m_sc_desc.Windowed = true;
	m_sc_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Allow switch mode through IDXGISwapChain::ResizeTarget (e.g Windowed to Fullscreen)
	m_sc_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// No we dont allow Mode Switch: We handle exclusive fullscreen manually :)
	//m_sc_desc.Flags = 0;

	HRCHECK(m_factory->CreateSwapChain(m_device.Get(), &m_sc_desc, m_sc.GetAddressOf()));

	// Setup viewport 
	m_bbViewport.TopLeftX = m_bbViewport.TopLeftY = 0;
	m_bbViewport.Width = static_cast<float>(bbWidth);
	m_bbViewport.Height = static_cast<float>(bbHeight);
	m_bbViewport.MinDepth = 0.f;
	m_bbViewport.MaxDepth = 1.f;
}

void DXDevice::create_bb_target()
{
	// ============= Create Backbuffer View
	HRCHECK(m_sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)m_bbTex.GetAddressOf()));

	// Gamma corrected when writing into backbuffer view
	// We cannot use SRGB on the SwapChain format since it's not allowed when using the new FLIP model
	// https://walbourn.github.io/care-and-feeding-of-modern-swapchains/
	// "sRGB gamma/de-gamma behavior is really an aspect of “interpreting the bits”, i.e. part of the view."
	// "When you read a pixel from your typical r8g8b8a8 texture, you need to remove the gamma curve, do all your lighting math, 
	// then apply the gamma curve as you write the pixel into your render target"
	// --> SRGB on SRV read automatically makes sure to de-gamma it before use
	// --> SRGB on RTV write automatically makes sure to gamma it before writing
	//D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;		// Automatically gamma correct result to our non-SRBG backbuffer
	//rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//HRCHECK(m_device->CreateRenderTargetView(m_bbTex.Get(), &rtvDesc, m_bbView.GetAddressOf()));

	// We will use default UNORM non-SRGB specified by Texture format (We gamma correct manually)
	HRCHECK(m_device->CreateRenderTargetView(m_bbTex.Get(), nullptr, m_bbView.GetAddressOf()));
}

HWND DXDevice::get_hwnd() const
{
	return m_hwnd;
}

const Device1Ptr& DXDevice::get_device() const
{
	return m_device_1;
}

const DeviceContext1Ptr& DXDevice::get_context() const
{
	return m_context_1;
}

const SwapChainPtr& DXDevice::get_sc() const
{
	return m_sc;
}

const Tex2DPtr& DXDevice::get_bb_texture() const
{
	return m_bbTex;
}

const RtvPtr& DXDevice::get_bb_target() const
{
	return m_bbView;
}

const D3D11_VIEWPORT& DXDevice::get_bb_viewport() const
{
	return m_bbViewport;
}

const AnnotationPtr DXDevice::get_annotation() const
{
	return m_annotation;
}

const DXGI_MODE_DESC& DXDevice::get_best_mode() const
{
	return m_available_display_modes.back();
}

void DXDevice::resize_swapchain(UINT width, UINT height)
{
	// Release texture and vview
	m_bbTex->Release();
	m_bbView->Release();

	auto best_mode = m_available_display_modes.back();
	best_mode.Width = width;
	best_mode.Height = height;

	// Update mode
	m_sc_desc.BufferDesc = best_mode;

	HRCHECK(m_sc->ResizeTarget(&best_mode));
	HRCHECK(m_sc->ResizeBuffers(s_buffer_count, width, height, best_mode.Format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	// Automatically choose the width and height to match the client rect for HWNDs
	//HRCHECK(m_sc->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));
	
	// Grab new texture and and recreate render target for it
	create_bb_target();
}

const DXGI_SWAP_CHAIN_DESC& DXDevice::get_sc_desc() const
{
	return m_sc_desc;
}

