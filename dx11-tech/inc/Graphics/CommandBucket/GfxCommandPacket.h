#pragma once
#include <memory>
#include "Memory/Allocator.h"

typedef void (*GfxCommandDispatch)(const void*);
typedef void* GfxCommandPacket;

/*
Memory layout:
    struct GfxCommandPacket
    {
        GfxCommandPacket* next_command;
        GfxCommandDispatch dispatch_function;
        T command
        char[] aux_memory       // Note that there is no safety: We DEFINE commands that either use or DONT use aux memory!
    }
*/
namespace gfxcommandpacket
{
    // Offsets defined by above memory layout
    static const size_t OFFSET_NEXT_PACKET = 0u;
    static const size_t OFFSET_DISPATCH = OFFSET_NEXT_PACKET + sizeof(GfxCommandPacket);
    static const size_t OFFSET_COMMAND = OFFSET_DISPATCH + sizeof(GfxCommandDispatch);

    template <typename T>
    GfxCommandPacket create(size_t aux_size, Allocator* allocator)
    {
        //return std::malloc(sizeof(GfxCommandPacket) + sizeof(GfxCommandDispatch) + sizeof(T) + aux_size);
        auto packet_size = sizeof(GfxCommandPacket) + sizeof(GfxCommandDispatch) + sizeof(T) + aux_size;
        return allocator->allocate(packet_size);
    }

    /*
        Helpers for getting data
    */

    GfxCommandPacket* get_next_packet(GfxCommandPacket packet);
    GfxCommandDispatch* get_dispatch(GfxCommandPacket packet);

    template <typename T>
    T* get_command(GfxCommandPacket packet)
    {
        return reinterpret_cast<T*>((char*)packet + OFFSET_COMMAND);
    }
    
    // Anonymous version of get_command
    const void* get_command_ptr(const GfxCommandPacket packet);

    template <typename T>
    char* get_aux_memory(T* command)
    {
        return (char*)command + sizeof(T);
    }

    /*
        Helpers for storing data
    */

    // append a packet to an existing [[packet]]
    void append_packet(GfxCommandPacket base, GfxCommandPacket next);

    // append a packet to an existing [[command]]
    template <typename T>
    void append_packet(T* command, GfxCommandPacket next)
    {
        // Go to beginning of Packet from command.
        *(reinterpret_cast<GfxCommandPacket*>((char*)command - OFFSET_COMMAND + OFFSET_NEXT_PACKET)) = next;
    }

    void store_dispatch(GfxCommandPacket packet, GfxCommandDispatch dispatchFunction);

}
