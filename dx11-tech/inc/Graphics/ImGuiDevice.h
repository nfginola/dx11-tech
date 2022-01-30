#pragma once

class ImGuiDevice
{
public:
	static void initialize(class GfxDevice* dev);
	static void shutdown();

	void begin_frame();
	void draw();
	void end_frame();

	bool win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	ImGuiDevice(class GfxDevice* dev);
	~ImGuiDevice();
};

