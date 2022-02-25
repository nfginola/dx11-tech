#pragma once

class Allocator
{
public:
	Allocator() {};
	virtual ~Allocator() {};
 
	virtual void* allocate(size_t size) = 0;
	virtual void deallocate(void* ptr) = 0;
	virtual void reset() = 0;
};
