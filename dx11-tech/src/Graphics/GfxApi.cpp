#include "pch.h"
#include "Graphics/GfxApi.h"
#include "Graphics/GfxCommon.h"

uint64_t get_next_id(uint64_t& counter)
{
	auto id = counter;
	++counter;
	return id;
}


GfxApi::GfxApi(std::unique_ptr<DXDevice> dev) :
	m_dev(std::move(dev))
{

}

GfxApi::~GfxApi()
{

}


