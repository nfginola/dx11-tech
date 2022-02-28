#include "pch.h"
#include "Memory/LinearAllocator.h"
#include <memory>
#include <cmath>
#include <assert.h>

LinearAllocator::LinearAllocator(size_t size) :
	m_internal_size(size)
{
	m_memory = (char*)std::malloc(size);
    std::memset(m_memory, 0, size);
	m_end = m_memory + m_internal_size;
}

LinearAllocator::~LinearAllocator()
{
	std::free(m_memory);
    m_memory = nullptr;
}

void* LinearAllocator::allocate(size_t size)
{

    // get start of block
    char* ret = m_memory + m_offset;

    // place at an 8 byte aligned address
    static constexpr uint8_t align_by = 8;
    uint8_t mod = (uint64_t)(ret) % align_by;
    uint8_t contrib = (uint8_t)std::ceilf((float)mod / align_by);      // If mod == 0, already aligned, don't apply math function to align (16 - 8 = 8, we'd like to keep 8)           
    uint64_t to_align_with = (align_by - mod) * contrib;
    ret += to_align_with;
    size += to_align_with;
    assert((uint64_t)ret % 8 == 0);

    //std::memset(ret, 0, size);

    // if start of block at end or further, no more memory available
    if (ret >= m_end)
        return nullptr;

    // if end of requested block is further than end, no memory available
    if ((ret + size) > m_end)
        return nullptr;

    // successful allocation
    m_offset += size;

    return ret;
}

void LinearAllocator::reset()
{
    m_offset = 0;
}
