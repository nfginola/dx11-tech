#pragma once



class Material
{
public:
	enum class Texture
	{
		// Phong
		eAlbedo,
		eNormal
	};

public:
	Material() = default;
	~Material() = default;

	bool operator==(const Material& other) const { return other.m_textures == m_textures; };

	Material& set_texture(Texture type, const class GPUTexture* texture);
	const GPUTexture* get_texture(Texture type) const;

private:
	std::map<Texture, const GPUTexture*> m_textures;

};

