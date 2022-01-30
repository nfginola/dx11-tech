#pragma once

class ImGuiDevice
{
public:
	static void initialize(class GfxDevice* dev);
	static void shutdown();

	void add_ui_callback(const std::string& name, std::function<void()> func);
	void remove_ui_callback(const std::string& name);

	void begin_frame();
	void draw();
	void end_frame();

	bool win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


private:
	void start_docking();

	ImGuiDevice(class GfxDevice* dev);
	~ImGuiDevice();

private:
	std::unordered_map<std::string, std::function<void()>> m_ui_callbacks;
};

