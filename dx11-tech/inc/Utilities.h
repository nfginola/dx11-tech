#pragma once

namespace utils
{
	std::string to_str(std::wstring str);
	std::wstring to_wstr(std::string str);
	std::vector<uint8_t> read_file(const std::filesystem::path& filePath);

	void constrained_incr(float& num, float min, float max);
	void constrained_decr(float& num, float min, float max);
	float constrained_add(float lh, float rh, float min, float max);

}

