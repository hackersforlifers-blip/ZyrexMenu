//============ Copyright ImMagic, All rights reserved ============//
//
// Purpose: 
//
//================================================================//

#pragma once

#include <Windows.h>

class CWindow
{
public:
	typedef LRESULT(*MessageHandler)(HWND, UINT, WPARAM, LPARAM);

public:
	CWindow();
	~CWindow();

public:
	bool Initialize(
#ifdef UNICODE
		const wchar_t* class_name, const wchar_t* wnd_title,
#else
		const char* class_name, const char* wnd_title,
#endif
		int pos_x, int pos_y, int width, int height, bool popup = false
	);

	void Release();

public:
	RECT GetWindowRect();
	HWND GetWindowHandle();

public:
	void SetWindowHandle(HWND hWnd);

#ifdef UNICODE
	void SetWindowTitle(const wchar_t* title);
#else
	void SetWindowTitle(const char* title);
#endif

	void SetWindowSize(float width, float height);
	void SetWindowPos(float x, float y);

public:
	void Show();
	void Hide();

	void Maximize();
	void Minimize();

public:
	// Default handler that gets called if you didn’t set a custom one
	static LRESULT DefaultHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Custom handler pointer
	MessageHandler HandleMessage;

private:
	HWND		m_hWnd;
	WNDCLASSEX	m_wc;
};