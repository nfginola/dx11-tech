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

	Material& set_texture(Texture type, const class GPUTexture* texture);
	const GPUTexture* get_texture(Texture type) const;

private:
	std::map<Texture, const GPUTexture*> m_textures;

};

