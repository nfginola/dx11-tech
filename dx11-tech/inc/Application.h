#pragma once
class Application
{
public:
	Application();
	~Application();

	Application& operator=(const Application&) = delete;
	Application(const Application&) = delete;

	void run();

private:
	bool m_paused = false;
	bool m_app_alive = true;

	unique_ptr<class Window> m_win;
	unique_ptr<class Input> m_input;

	LRESULT custom_win_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};

