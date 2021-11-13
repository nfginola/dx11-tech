#include "pch.h"
#include "Graphics/DXDevice.h"
#include "Graphics/dx.h"

dx* dx::s_self = nullptr;

dx::dx(shared_ptr<DXDevice> dev) :
	m_dev(dev)
{

}

void dx::init(shared_ptr<DXDevice> dev)
{
	s_self = new dx(dev);
}

void dx::shutdown()
{
	delete s_self;
}

dx* dx::get()
{
	if (!s_self)
		assert(false);
	return s_self;
}

BufferID dx::create_vertex_buffer()
{
	std::cout << "creating vb\n";
	return { (uint64_t)rand() };
}

BufferID dx::create_index_buffer()
{
	std::cout << "creating ib\n";
	return { (uint64_t)rand() };

}

BufferID dx::create_buffer()
{
	std::cout << "create generic buffer\n";
	return { (uint64_t)rand() };
}

TextureID dx::create_texture()
{
	std::cout << "create generic texture\n";
	return { (uint64_t)rand() };
}

void dx::bind_buf(BufferID id)
{
	std::cout << "bound buffer " << id << std::endl;
}

void dx::bind_tex(TextureID id)
{
	std::cout << "bound texture " << id << std::endl;
}
