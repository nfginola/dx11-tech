#include "pch.h"
#include "Graphics/CommandBucket/GfxCommandPacket.h"


const void* gfxcommandpacket::get_command_ptr(const GfxCommandPacket packet)
{
    return reinterpret_cast<char*>(packet) + OFFSET_COMMAND;
}

void gfxcommandpacket::append_packet(GfxCommandPacket base, GfxCommandPacket next)
{
    *get_next_packet(base) = next;
}

void gfxcommandpacket::store_dispatch(GfxCommandPacket packet, GfxCommandDispatch dispatchFunction)
{
    *get_dispatch(packet) = dispatchFunction;
}

GfxCommandPacket* gfxcommandpacket::get_next_packet(GfxCommandPacket packet)
{
    return reinterpret_cast<GfxCommandPacket*>((char*)packet + OFFSET_NEXT_PACKET);
}

GfxCommandDispatch* gfxcommandpacket::get_dispatch(GfxCommandPacket packet)
{
    return reinterpret_cast<GfxCommandDispatch*>((char*)packet + OFFSET_DISPATCH);
}
