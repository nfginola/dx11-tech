#include "pch.h"
#include "Utilities.h"

namespace utils
{
	std::string to_str(std::wstring wstr)
	{
		if (wstr.empty()) return std::string();
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string str_res(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str_res[0], size_needed, NULL, NULL);
		return str_res;
	}
	std::wstring to_wstr(std::string str)
	{
		if (str.empty()) return std::wstring();
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
		std::wstring wstr_res(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr_res[0], size_needed);
		return wstr_res;
	}

	std::vector<uint8_t> read_file(const std::filesystem::path& filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
			assert(false);

		size_t file_size = static_cast<size_t>(file.tellg());
		std::vector<uint8_t> buffer(file_size);

		file.seekg(0);
		file.read((char*)buffer.data(), file_size);

		file.close();
		return buffer;
	}

	void constrained_incr(float& num, float min, float max)
	{
		++num;
		num = (std::max)((std::min)(num, max), min);
	}

	void constrained_decr(float& num, float min, float max)
	{
		--num;
		num = (std::max)((std::min)(num, max), min);
	}

	float constrained_add(float lh, float rh, float min, float max)
	{
		lh += rh;
		lh = (std::max)((std::min)(lh, max), min);
		return lh;
	}
}
