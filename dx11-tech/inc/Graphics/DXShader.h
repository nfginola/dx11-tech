#pragma once
#include "Graphics/DXDevice.h"

class DXShader
{
public:
	struct Module
	{
		ShaderStage stage;
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
	~DXShader() = default;

	void recompile(DXDevice* dev);

private:
	VsPtr m_vs;
	HsPtr m_hs;
	DsPtr m_ds;
	GsPtr m_gs;
	PsPtr m_ps;

	std::vector<Module> m_modules;

	bool m_recompilation_allowed = true;

private:
	void create(DXDevice* dev);


};

