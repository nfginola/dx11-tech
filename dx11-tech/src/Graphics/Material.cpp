#include "pch.h"
#include "Graphics/Material.h"

Material& Material::set_texture(Material::Texture type, TextureHandle tex)
{
	m_textures.insert({ type, tex });
	return *this;
}

TextureHandle Material::get_texture(Material::Texture type) const
{
	auto it = m_textures.find(type);
	if (it == m_textures.cend())
		return TextureHandle{0};
	return it->second;
}
