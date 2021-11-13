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

void dx::create_vertex_buffer()
{
	std::cout << "creating vb\n";
}

void dx::create_index_buffer()
{
	std::cout << "creating ib\n";

}

void dx::create_buffer()
{
	std::cout << "create generic buffer\n";
}

void dx::create_texture()
{
	std::cout << "create generic texture\n";
}
