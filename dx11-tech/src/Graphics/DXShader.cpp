#include "pch.h"
#include "Graphics/DXDevice.h"
#include "Graphics/DXShader.h"

// Static definitions
std::unordered_map<ShaderStage, std::pair<std::string, std::string>> DXShader::s_hlsl_compile_lookup;
bool DXShader::s_lookup_initialized = false;

DXShader::DXShader(DXDevice* dev,
	const std::filesystem::path& vs_path, 
	const std::filesystem::path& ps_path,
	const std::filesystem::path& gs_path,
	const std::filesystem::path& hs_path,
	const std::filesystem::path& ds_path)
{
	init_lookup();
	m_recompilation_allowed = sanitize(vs_path, ps_path, gs_path, hs_path, ds_path);
	
	// Prepare modules
	if (!vs_path.empty())	m_modules.push_back(prepare_module(vs_path, ShaderStage::Vertex));
	if (!ps_path.empty())	m_modules.push_back(prepare_module(ps_path, ShaderStage::Pixel));
	if (!gs_path.empty())	m_modules.push_back(prepare_module(gs_path, ShaderStage::Geometry));
	if (!hs_path.empty())	m_modules.push_back(prepare_module(hs_path, ShaderStage::Hull));
	if (!ds_path.empty())	m_modules.push_back(prepare_module(ds_path, ShaderStage::Domain));

#ifdef SHADER_DEBUG_NO_FILE
	std::cout << "SHADER: Running Test Version: No files read\n";
#else
	create(dev);
#endif
}

DXShader::DXShader(DXDevice* dev, const std::filesystem::path& cs_path)
{

	assert(false);
	// to implement: compute
}

void DXShader::bind(DXDevice* dev)
{
	if (m_vs)
		dev->get_context()->VSSetShader(m_vs.Get(), NULL, 0);
	else
		dev->get_context()->VSSetShader(nullptr, NULL, 0);

	if (m_ps)
		dev->get_context()->PSSetShader(m_ps.Get(), NULL, 0);
	else
		dev->get_context()->PSSetShader(nullptr, NULL, 0);

	if (m_gs)
		dev->get_context()->GSSetShader(m_gs.Get(), NULL, 0);
	else
		dev->get_context()->GSSetShader(nullptr, NULL, 0);

	if (m_ds)
		dev->get_context()->DSSetShader(m_ds.Get(), NULL, 0);
	else
		dev->get_context()->DSSetShader(nullptr, NULL, 0);

	if (m_hs)
		dev->get_context()->HSSetShader(m_hs.Get(), NULL, 0);
	else
		dev->get_context()->HSSetShader(nullptr, NULL, 0);


	if (m_cs)
		dev->get_context()->CSSetShader(m_cs.Get(), NULL, 0);
}

void DXShader::recompile(DXDevice* dev)
{
	if (!m_recompilation_allowed)
	{
		std::cout << "recompilation not allowed\n";
		assert(false);
	}

#ifdef SHADER_DEBUG_NO_FILE
#else
	for (auto& mod : m_modules)
		mod.code = utils::read_file(mod.uncompiled_path);

	create(dev);
#endif
}

void DXShader::create(DXDevice* dev)
{
	for (const auto& mod : m_modules)
		mod.create_func(dev->get_device(), mod.code);
}

std::vector<uint8_t> DXShader::load_shader_blob(const std::filesystem::path& path, ShaderStage shader_type)
{
	ComPtr<ID3DBlob> error_blob;
	ComPtr<ID3DBlob> compiled;

	auto file_name = utils::to_wstr(path.string());
	auto shader_type_and_ver = s_hlsl_compile_lookup[shader_type].first;
	auto entry_point = s_hlsl_compile_lookup[shader_type].second;

	HRESULT hr = D3DCompileFromFile(
		file_name.c_str(),
		NULL,
		NULL,
		entry_point.c_str(),
		shader_type_and_ver.c_str(),
		0,
		0,
		compiled.GetAddressOf(),
		error_blob.GetAddressOf()
	);

	if (FAILED(hr))
	{
		OutputDebugStringA((char*)error_blob->GetBufferPointer());
		assert(false);
	}

	size_t compiled_size = static_cast<size_t>(compiled->GetBufferSize());
	std::vector<uint8_t> buffer(compiled_size);

	std::memcpy(buffer.data(), compiled->GetBufferPointer(), compiled_size);
	return buffer;
}

void DXShader::init_lookup()
{
	if (!s_lookup_initialized)
	{
		s_hlsl_compile_lookup.insert({ ShaderStage::Vertex, { "vs_5_0", "VS_MAIN" } });
		s_hlsl_compile_lookup.insert({ ShaderStage::Pixel, { "ps_5_0", "PS_MAIN" } });
		s_hlsl_compile_lookup.insert({ ShaderStage::Geometry, { "gs_5_0", "GS_MAIN" } });
		s_hlsl_compile_lookup.insert({ ShaderStage::Hull, { "hs_5_0", "HS_MAIN"} });
		s_hlsl_compile_lookup.insert({ ShaderStage::Domain, { "ds_5_0", "DS_MAIN"} });
		s_hlsl_compile_lookup.insert({ ShaderStage::Compute, { "cs_5_0", "CS_MAIN"} });
		s_lookup_initialized = true;
	}
}

bool DXShader::sanitize(
	const std::filesystem::path& vs_path,
	const std::filesystem::path& ps_path,
	const std::filesystem::path& gs_path,
	const std::filesystem::path& hs_path,
	const std::filesystem::path& ds_path)
{
	if (vs_path.empty() || ps_path.empty())
		assert(false);
		
	bool recompilation_allowed = false;
	/*
		We allow only GROUPS of either uncompiled (.hlsl) or compiled (.cso) shaders
	*/
	if (vs_path.extension() == ".cso" &&
		((!hs_path.empty() && hs_path.extension() == ".cso") || hs_path.empty()) &&		// if empty, we let it pass as it has no contribution
		((!ds_path.empty() && ds_path.extension() == ".cso") || ds_path.empty()) &&
		((!gs_path.empty() && gs_path.extension() == ".cso") || gs_path.empty()) &&
		ps_path.extension() == ".cso")
	{
		recompilation_allowed = false;
		std::cout << "recompilation not allowed (.cso)" << std::endl;
	}
	else if (
		vs_path.extension() == ".hlsl" &&
		((!hs_path.empty() && hs_path.extension() == ".hlsl") || hs_path.empty()) &&
		((!ds_path.empty() && ds_path.extension() == ".hlsl") || ds_path.empty()) &&
		((!gs_path.empty() && gs_path.extension() == ".hlsl") || gs_path.empty()) &&
		ps_path.extension() == ".hlsl")
	{
		recompilation_allowed = true;
		std::cout << "recompilation allowed (.hlsl)" << std::endl;
	}
	else
	{
		std::cout << "non matching shader extensions!" << std::endl;
		assert(false);
	}

	return recompilation_allowed;
}

DXShader::Module DXShader::prepare_module(const std::filesystem::path& path, ShaderStage stage)
{
	Module mod;
	mod.stage = stage;
	mod.uncompiled_path = m_recompilation_allowed ? path : "";

#ifdef SHADER_DEBUG_NO_FILE
	mod.code = std::vector<uint8_t>();
#else
	mod.code = m_recompilation_allowed ? load_shader_blob(path, stage) : utils::read_file(path);
#endif

	switch (stage)
	{
	case ShaderStage::Vertex:
		mod.create_func = [this](const DevicePtr& dev, const std::vector<uint8_t>& code)
		{
			dev->CreateVertexShader(code.data(), code.size(), nullptr, this->m_vs.GetAddressOf());
		};
		break;
	case ShaderStage::Pixel:
		mod.create_func = [this](const DevicePtr& dev, const std::vector<uint8_t>& code)
		{
			dev->CreatePixelShader(code.data(), code.size(), nullptr, this->m_ps.GetAddressOf());
		};
		break;
	case ShaderStage::Geometry:
		mod.create_func = [this](const DevicePtr& dev, const std::vector<uint8_t>& code)
		{
			dev->CreateGeometryShader(code.data(), code.size(), nullptr, this->m_gs.GetAddressOf());
		};
		break;
	case ShaderStage::Hull:
		mod.create_func = [this](const DevicePtr& dev, const std::vector<uint8_t>& code)
		{
			dev->CreateHullShader(code.data(), code.size(), nullptr, this->m_hs.GetAddressOf());
		};
		break;
	case ShaderStage::Domain:
		mod.create_func = [this](const DevicePtr& dev, const std::vector<uint8_t>& code)
		{
			dev->CreateDomainShader(code.data(), code.size(), nullptr, this->m_ds.GetAddressOf());
		};
		break;
	case ShaderStage::Compute:
		mod.create_func = [this](const DevicePtr& dev, const std::vector<uint8_t>& code)
		{
			dev->CreateComputeShader(code.data(), code.size(), nullptr, this->m_cs.GetAddressOf());
		};
		break;
	default:
		assert(false);
	}

	return mod;
}
