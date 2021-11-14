#pragma once
#include "Graphics/DXCommon.h"
#include <unordered_map>

#define SHADER_DEBUG_NO_FILE

class DXShader
{
public:
	struct Module
	{
		ShaderStage stage = ShaderStage::Invalid;
		std::filesystem::path uncompiled_path = "";
		std::vector<uint8_t> code;
		std::function<void(const DevicePtr&, const std::vector<uint8_t>&)> create_func;
	};

public:
	DXShader(DXDevice* dev,
		const std::filesystem::path& vs_path,
		const std::filesystem::path& ps_path,
		const std::filesystem::path& gs_path = "",
		const std::filesystem::path& hs_path = "",
		const std::filesystem::path& ds_path = "");

	DXShader(DXDevice* dev,
		const std::filesystem::path& cs_path);

	~DXShader() = default;

	void bind(DXDevice* dev);
	void recompile(DXDevice* dev);

private:
	static bool s_lookup_initialized;
	static std::unordered_map<ShaderStage, std::pair<std::string, std::string>> s_hlsl_compile_lookup;

	VsPtr m_vs;
	HsPtr m_hs;
	DsPtr m_ds;
	GsPtr m_gs;
	PsPtr m_ps;
	
	CsPtr m_cs;

	std::vector<Module> m_modules;

	bool m_recompilation_allowed = true;

private:
	void create(DXDevice* dev);
	std::vector<uint8_t> load_shader_blob(const std::filesystem::path& path, ShaderStage shader_type);

	void init_lookup();
	bool sanitize(
		const std::filesystem::path& vs_path,
		const std::filesystem::path& ps_path,
		const std::filesystem::path& gs_path,
		const std::filesystem::path& hs_path,
		const std::filesystem::path& ds_path);

	Module prepare_module(const std::filesystem::path& path, ShaderStage stage);

};

