#pragma once
#include "Graphics/API/GfxHandles.h"


class Material
{
public:
	enum class Texture
	{
		eAlbedo,
		eNormal
	};

public:
	Material() = default;
	~Material() = default;

	bool operator==(const Material& other) const { return other.m_textures == m_textures; };

	Material& set_texture(Texture type, TextureHandle tex);
	TextureHandle get_texture(Texture type) const;

private:
	std::map<Texture, TextureHandle> m_textures;

};

