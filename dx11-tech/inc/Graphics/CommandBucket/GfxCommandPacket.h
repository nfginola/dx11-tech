#pragma once
#include <memory>

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
    // Offsets
    static const size_t OFFSET_NEXT_PACKET = 0u;
    static const size_t OFFSET_DISPATCH = OFFSET_NEXT_PACKET + sizeof(GfxCommandPacket);
    static const size_t OFFSET_COMMAND = OFFSET_DISPATCH + sizeof(GfxCommandDispatch);

    //template <typename T, typename Allocator>
    //GfxCommandPacket create(size_t aux_size)
    //{
    //    return Allocator::allocate(sizeof(GfxCommandPacket) + sizeof(GfxCommandDispatch) + sizeof(T) + aux_size);
    //}

    template <typename T>
    GfxCommandPacket create(size_t aux_size)
    {
        return std::malloc(sizeof(GfxCommandPacket) + sizeof(GfxCommandDispatch) + sizeof(T) + aux_size);
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

    void append_packet(GfxCommandPacket base, GfxCommandPacket next);
    void store_dispatch(GfxCommandPacket packet, GfxCommandDispatch dispatchFunction);

    //const CommandPacket LoadNextCommandPacket(const CommandPacket packet)
    //{
    //    return *GetNextCommandPacket(packet);
    //}

    //const BackendDispatchFunction LoadBackendDispatchFunction(const  CommandPacket packet)
    //{
    //    return *GetBackendDispatchFunction(packet);
    //}

    //const void* LoadCommand(const CommandPacket packet)
    //{
    //    return reinterpret_cast<char*>(packet) + OFFSET_COMMAND;
    //}

}
