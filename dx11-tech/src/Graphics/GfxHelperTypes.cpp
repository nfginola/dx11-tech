#include "pch.h"
#include "Graphics/GfxHelperTypes.h"


void GPUAnnotator::begin_event(const std::string& name)
{
    m_annotation->BeginEvent(utils::to_wstr(name).c_str());
}

void GPUAnnotator::end_event()
{
    m_annotation->EndEvent();
}

void GPUAnnotator::set_marker(const std::string& name)
{
    m_annotation->SetMarker(utils::to_wstr(name).c_str());
}
