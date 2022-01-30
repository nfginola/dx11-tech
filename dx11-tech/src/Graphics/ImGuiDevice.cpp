#include "pch.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "Graphics/ImGuiDevice.h"
#include "Graphics/GfxDevice.h"


namespace gfx
{
    ImGuiDevice* imgui = nullptr;
}


void ImGuiDevice::initialize(GfxDevice* dev)
{
    if (!gfx::imgui)
        gfx::imgui = new ImGuiDevice(dev);
}

void ImGuiDevice::shutdown()
{
    if (gfx::imgui)
    {
        delete gfx::imgui;
        gfx::imgui = nullptr;
    }
}

void ImGuiDevice::start_docking()
{
    // Make dockspace over the whole viewport
    // This has to be called prior to all other ImGUI calls
    ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;      // Pass through the otherwise background cleared window
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), flags);
}

ImGuiDevice::ImGuiDevice(GfxDevice* dev)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();  (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    //io.ConfigDockingWithShift = true;   // Must hold shift to dock windows

    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(dev->m_dev->get_hwnd());
    ImGui_ImplDX11_Init(dev->m_dev->get_device().Get(), dev->m_dev->get_context().Get());
}

ImGuiDevice::~ImGuiDevice()
{
    ImGui_ImplWin32_Shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiDevice::begin_frame()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    start_docking();
}

void ImGuiDevice::draw()
{
    // Rendering
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiDevice::end_frame()
{
    ImGuiIO& io = ImGui::GetIO();

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool ImGuiDevice::win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;
    return false;
}

