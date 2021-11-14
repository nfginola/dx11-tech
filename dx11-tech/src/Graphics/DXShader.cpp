#include "pch.h"
#include "Graphics/DXDevice.h"
#include "Graphics/DXShader.h"

DXShader::DXShader(DXDevice* dev,
	const std::filesystem::path& vs_path, 
	const std::filesystem::path& ps_path,
	const std::filesystem::path& gs_path,
	const std::filesystem::path& hs_path,
	const std::filesystem::path& ds_path)
{
	if (vs_path.empty() || ps_path.empty())
		assert(false);

	/*
		We allow groups of either uncompiled (.hlsl) or compiled (.cso) shaders
	*/
	if (vs_path.extension() == ".cso" &&
		((!hs_path.empty() && hs_path.extension() == ".cso") || hs_path.empty()) &&		// if empty, we let it pass as it has no contribution
		((!ds_path.empty() && ds_path.extension() == ".cso") || ds_path.empty()) &&
		((!gs_path.empty() && gs_path.extension() == ".cso") || gs_path.empty()) &&
		ps_path.extension() == ".cso")
	{
		m_recompilation_allowed = false;
		std::cout << "recompilation not allowed (.cso)" << std::endl;
	}
	else if (
		vs_path.extension() == ".hlsl" &&
		((!hs_path.empty() && hs_path.extension() == ".hlsl") || hs_path.empty()) &&
		((!ds_path.empty() && ds_path.extension() == ".hlsl") || ds_path.empty()) &&
		((!gs_path.empty() && gs_path.extension() == ".hlsl") || gs_path.empty()) &&
		ps_path.extension() == ".hlsl")
	{
		m_recompilation_allowed = true;
		std::cout << "recompilation allowed (.hlsl)" << std::endl;
	}
	else
	{
		std::cout << "non matching shader extensions!" << std::endl;
		assert(false);
	}

	//// Vertex
	//{
	//	Module mod;
	//	mod.stage = Stage::Vertex;
	//	mod.code = m_recompilation_allowed ? std::vector<uint8_t>() : utils::read_file(vs_path);
	//	mod.uncompiled_path = m_recompilation_allowed ? vs_path : "";
	//	mod.create_func = [this](const DevicePtr& dev, const std::vector<uint8_t>& code)
	//	{
	//		dev->CreateVertexShader(code.data(), code.size(), nullptr, this->m_vs.GetAddressOf());
	//	};
	//	m_modules.push_back(mod);
	//}

	//// Pixel
	//{
	//	Module mod;
	//	mod.stage = Stage::Pixel;
	//	mod.code = m_recompilation_allowed ? std::vector<uint8_t>() : utils::read_file(vs_path);
	//  mod.uncompiled_path = m_recompilation_allowed ? ps_path : "";
	//	mod.create_func = [this](const DevicePtr& dev, const std::vector<uint8_t>& code)
	//	{
	//		dev->CreatePixelShader(code.data(), code.size(), nullptr, this->m_ps.GetAddressOf());
	//	};
	//	m_modules.push_back(mod);
	//}

	create(dev);
}

void DXShader::recompile(DXDevice* dev)
{
	if (!m_recompilation_allowed)
	{
		std::cout << "recompilation not allowed\n";
		assert(false);
	}

	for (auto& mod : m_modules)
		mod.code = utils::read_file(mod.uncompiled_path);

	create(dev);
}

void DXShader::create(DXDevice* dev)
{
	for (const auto& mod : m_modules)
		mod.create_func(dev->get_device(), mod.code);
}
