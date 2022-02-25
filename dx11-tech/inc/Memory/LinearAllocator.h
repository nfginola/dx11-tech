#pragma once
#include "Memory/Allocator.h"

class LinearAllocator : public Allocator
{
public:
	LinearAllocator() = delete;
	LinearAllocator(size_t size);
	~LinearAllocator();

	void* allocate(size_t size) override;
	void deallocate(void* ptr) override { };
	void reset() override;

private:
	char* m_memory = nullptr;
	size_t m_internal_size = 0;
	char* m_end = nullptr;

	// "Where is the next allocation at from base?"
	size_t m_offset = 0;

	
};
