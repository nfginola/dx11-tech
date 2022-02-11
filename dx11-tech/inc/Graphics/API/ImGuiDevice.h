#pragma once

class ImGuiDevice
{
public:
	static void initialize(class GfxDevice* dev);
	static void shutdown();

	/*
		In the spirit of using ImGUI in general,
		e.g ImGui::Begin(...) using static functions,
		we will also have a static interface for defining UI callbacks
	*/
	static void add_ui(const std::string& name, std::function<void()> func);
	static void remove_ui(const std::string& name);

	void begin_frame();
	void draw();
	void end_frame();

	bool win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


private:
	void add_ui_callback(const std::string& name, std::function<void()> func);
	void remove_ui_callback(const std::string& name);

	void start_docking();

	ImGuiDevice(class GfxDevice* dev);
	~ImGuiDevice();

private:
	std::unordered_map<std::string, std::function<void()>> m_ui_callbacks;
};

