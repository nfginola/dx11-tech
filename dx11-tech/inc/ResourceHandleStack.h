#pragma once

#include <stdint.h>
#include <limits>
#include <vector>
#include <assert.h>

// Comment this out to use 32-bit handle. 
#define USE_64_BIT_RES_HANDLE

// Do NOT change the value. You are free to change the name if any collision arises in your program.
static constexpr uint8_t RES_INVALID_HANDLE = 0;

// Use res_handle in application code
#ifdef USE_64_BIT_RES_HANDLE
using res_handle = uint64_t;
#else
using res_handle = uint32_t;
#endif

// Uses ~2 MB when max_usable_elements = UINT16_MAX - 1 (default)
template <typename T, uint64_t max_usable_elements = std::numeric_limits<uint16_t>::max() - 1>
class ResourceHandleStack
{
private:
#ifdef USE_64_BIT_RES_HANDLE
	// 64-bit keys
	using half_key = uint32_t;
	using full_key = uint64_t;
#else
	// 32-bit keys
	using half_key = uint16_t;
	using full_key = uint32_t;
#endif

	static constexpr full_key INDEX_SHIFT = std::numeric_limits<half_key>::digits;
	static constexpr full_key SLOT_MASK = ((full_key)1 << INDEX_SHIFT) - 1;

	/*
		Resource is REQUIRED to have a
			- .free() function for deallocating the underlying resource
			- A stored "handle" which is used to validate for uniqueness upon usage
				- Handle = 0 indicates INVALID resource!
				- Should be as large as 'full_key'

		Refer to the called functions on the underlying resource in the 'free_handle(..)' function.
	*/

public:
	ResourceHandleStack()
	{
		assert(max_usable_elements > 3);

		// More indices than the key allows (half_key) is not allowed!
		assert(max_usable_elements <= std::numeric_limits<half_key>::max() - 1);

		// check static constexprs ( breakpoint here to check, they are not visible in debugger :/ )
		//auto tmp1 = INDEX_SHIFT;
		//auto tmp2 = SLOT_MASK;

		resources.resize(total_elements);
		free_indices.resize(total_elements);

		// Reserve 0 for invalid handle
		for (uint16_t i = 1; i < total_elements; ++i)
			free_indices[i] = i;

		gen_counter.resize(max_usable_elements + 1);
		std::fill(gen_counter.begin(), gen_counter.end(), 0);

		slots_enabled.resize(total_elements);
		slots_enabled[0] = false;	// reserved
		std::fill(slots_enabled.begin() + 1, slots_enabled.end(), true);
	}
	~ResourceHandleStack() = default;

	// Get next free handle
	std::pair<full_key, T*> get_next_free_handle()
	{
		//assert(top < total_elements);
		if (top >= total_elements)	// out of memory since top is beyond the stack now (index out of range)
			return { 0, nullptr };

		// get next free index from top of stack
		// should be a guarantee that no disabled slots are ever popped from stack and received here
		// (due to the fact that we never push a disabled slot onto the stack upon freeing)
		half_key idx = free_indices[top++];

		// get next generation counter for this index
		half_key ctr = gen_counter[idx]++;

		// disable slot when there are no unique patterns left (overflow)
		if (gen_counter[idx] < ctr)
			slots_enabled[idx] = false;

		// calculate handle (higher bits reserved for generational counter, lower bits for index)
		full_key hdl = (((full_key)ctr) << INDEX_SHIFT) | (((full_key)idx) & SLOT_MASK);

		return { hdl, &resources[idx] };
	}

	// Free the handle AND free the underlying resource
	void free_handle(full_key hdl)
	{
		half_key idx = (half_key)(hdl & SLOT_MASK);

		// handle free-after-free
		assert(resources[idx].handle == hdl);

		// free resource
		resources[idx].free();
		resources[idx].handle = 0;

		/*
			observe that we always prioritize re-using freed indices over new indices using the stack.
			we always try to exhaust a slots unique pattern.
		*/

		// put back index to top of stack only if slot is still enabled
		if (slots_enabled[idx])
			free_indices[--top] = idx;
		/*
			otherwise, top position is kept thus valid indices are kept on top.
			there will never be a disabled slot in the stack of free indices.
		*/
	}

	// Get the underlying resource
	T* look_up(full_key hdl)
	{
		half_key idx = (half_key)(hdl & SLOT_MASK);

		// check that we are in range
		assert(idx < total_elements&& hdl > 0);

		// check that the buffer handle is identical to the one at index
		// handle use-after-free
		assert(resources[idx].handle == hdl);

		return &resources[idx];
	}

	uint64_t get_memory_footprint()
	{
		return total_resource_bytes + total_bookkeeping_bytes + 3 * sizeof(uint64_t) + sizeof(half_key);
	}

private:
	// Total number of elements including the reserved 0 index
	// Stored as a member variable just so we can easily check size in debugger
	const uint64_t total_elements = max_usable_elements + 1;
	const uint64_t total_resource_bytes = total_elements * sizeof(T);
	const uint64_t total_bookkeeping_bytes = total_elements * (sizeof(half_key) * 2 + sizeof(bool));

	// Always assumes that 0 is an invalid handle
	half_key top = 1;

	std::vector<T> resources;
	std::vector<half_key> free_indices;
	std::vector<half_key> gen_counter;
	std::vector<bool> slots_enabled;
};
