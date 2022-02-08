#include "pch.h"
#include "Graphics/Material.h"

Material& Material::set_texture(Material::Texture type, const GPUTexture* texture)
{
	m_textures.insert({ type, texture });
	return *this;
}

const GPUTexture* Material::get_texture(Material::Texture type) const
{
	auto it = m_textures.find(type);
	if (it == m_textures.cend())
		return nullptr;
	return it->second;
}
