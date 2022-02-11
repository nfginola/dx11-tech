#pragma once
#include "Graphics/Model.h"

class ModelManager
{
public:
	static void initialize(class GfxDevice* dev, class MaterialManager* disk_tex_mgr);
	static void shutdown();

	ModelManager() = delete;
	~ModelManager() = default;

	const Model* load_model(const std::filesystem::path& path, const std::string& name = "");
	const Model* get_model(const std::string& name);
	void remove_model(const std::string& name);

private:
	ModelManager(GfxDevice* dev, MaterialManager* mat_mgr);

private:
	GfxDevice* m_dev = nullptr;
	MaterialManager* m_mat_mgr = nullptr;

	uint64_t m_def_counter = 0;
	std::map<std::filesystem::path, std::string> m_path_mapper;
	std::map<std::string, Model> m_models;

};

