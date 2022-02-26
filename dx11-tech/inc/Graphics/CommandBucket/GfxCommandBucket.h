#pragma once
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <execution>

#include "Graphics/CommandBucket/GfxCommandPacket.h"
#include "Memory/LinearAllocator.h"

template <typename T>
class GfxCommandBucket
{
    using Key = T;

    struct key_packet_pair
    {
        Key key;
        void* packet;
    };



public:
    GfxCommandBucket();

    template <typename U>
    U* add_command(Key key, size_t auxMemorySize);

    template <typename U, typename V>
    U* append_command(V* command, size_t auxMemorySize);

    void sort();
    void flush();

private:


private:
    static constexpr int max_cmds = 10000;

    void* m_key_packet_pairs;               // using vector adds some overhead on submission!
    //std::vector<key_packet_pair> m_key_packet_pairs;

    LinearAllocator m_packet_allocator;

    uint32_t m_current = 0;
};


template<typename T>
inline GfxCommandBucket<T>::GfxCommandBucket() :
    m_packet_allocator(1000000)
{
    //m_key_packet_pairs = program_mem_pool::grab_memory(max_cmds * sizeof(key_packet_pair));
    m_key_packet_pairs = std::malloc(max_cmds * sizeof(key_packet_pair));
    std::memset(m_key_packet_pairs, 0, max_cmds * sizeof(m_key_packet_pairs));
}

template<typename T>
inline void GfxCommandBucket<T>::sort()
{
    key_packet_pair* begin = (key_packet_pair*)m_key_packet_pairs;
    key_packet_pair* end = ((key_packet_pair*)m_key_packet_pairs) + m_current;
    //auto lookinto1 = (key_packet_pair(*)[250])m_key_packet_pairs;
    std::sort(std::execution::par_unseq, begin, end, [](const key_packet_pair& p1, const key_packet_pair& p2) { return p1.key > p2.key; });
}

template<typename T>
inline void GfxCommandBucket<T>::flush()
{
    for (uint32_t i = 0; i < m_current; ++i)
    {
        const auto& p = ((key_packet_pair*)m_key_packet_pairs)[i];
        GfxCommandPacket packet = p.packet;

        do
        {
            // Observe that we don't have the type information here, which is why
            // we need to assign a function pointer when adding command!
            auto func = *gfxcommandpacket::get_dispatch(packet);
            auto cmd = gfxcommandpacket::get_command_ptr(packet);
            func(cmd);

            packet = *gfxcommandpacket::get_next_packet(packet);
        } while (packet != nullptr);
    }

    m_packet_allocator.reset();
    m_current = 0;
}


template<typename T>
template<typename U>
inline U* GfxCommandBucket<T>::add_command(Key key, size_t aux_size)
{
    assert(m_current <= max_cmds);

    GfxCommandPacket packet = gfxcommandpacket::create<U>(aux_size, &m_packet_allocator);
    assert(packet != nullptr);
       
    const unsigned int current = m_current++;
    ((key_packet_pair*)m_key_packet_pairs)[current].key = key;
    ((key_packet_pair*)m_key_packet_pairs)[current].packet = packet;

    gfxcommandpacket::append_packet(packet, nullptr);           // Set next to nullptr
    gfxcommandpacket::store_dispatch(packet, U::DISPATCH);

    return gfxcommandpacket::get_command<U>(packet);
}

template<typename T>
template<typename U, typename V>
inline U* GfxCommandBucket<T>::append_command(V* base_command, size_t aux_size)
{
    /*
        Appends a command such that:
            base_command --> U 
    */
    GfxCommandPacket packet = gfxcommandpacket::create<U>(aux_size);
    
    // Assign defaults to new packet
    gfxcommandpacket::append_packet(packet, nullptr);
    gfxcommandpacket::store_dispatch(packet, U::DISPATCH);

    // Append this command to the given one
    gfxcommandpacket::append_packet<V>(base_command, packet);

    return gfxcommandpacket::get_command<U>(packet);
}




