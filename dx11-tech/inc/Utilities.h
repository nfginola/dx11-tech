#pragma once

namespace Utils
{
	std::string to_str(std::wstring str);
	std::wstring to_wstr(std::string str);
	std::vector<uint8_t> read_file(const std::filesystem::path& filePath);
}

