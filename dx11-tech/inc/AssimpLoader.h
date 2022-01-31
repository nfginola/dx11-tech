#pragma once

class AssimpLoader
{
public:
	AssimpLoader() = delete;
	AssimpLoader(const std::filesystem::path& fpath);
	
	/*
		Data are returned in non-interleaved form
		Packing to interleaved form is up to the end user
	*/
	void get_positions();
	void get_uvs();
	void get_normals();
	// to be extended

	template <typename T>
	T get_materials();

private:

};

