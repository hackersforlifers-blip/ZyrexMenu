//============ Copyright ImMagic, All rights reserved ============//
//
// Purpose: 
//
//================================================================//

#include "Window.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

CWindow::CWindow() : m_hWnd(NULL), HandleMessage(&CWindow::DefaultHandler)
{
	memset(&m_wc, 0, sizeof(WNDCLASSEX));
}

CWindow::~CWindow()
{
}

bool CWindow::Initialize(
#ifdef UNICODE
	const wchar_t* class_name, const wchar_t* wnd_title,
#else
	const char* class_name, const char* wnd_title,
#endif
	int pos_x, int pos_y, int width, int height,
	bool popup
)
{
	m_wc.cbSize = sizeof(WNDCLASSEX);
	m_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	m_wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	m_wc.hInstance = GetModuleHandle(nullptr);
	m_wc.lpszClassName = class_name;
	m_wc.lpszMenuName = "";
	m_wc.style = NULL;
	m_wc.lpfnWndProc = &WndProc;

	if (!::RegisterClassEx(&m_wc)) {
		return false;
	}

	DWORD dwStyle = popup ? WS_POPUP : WS_OVERLAPPEDWINDOW;

	m_hWnd = ::CreateWindowEx(
		NULL,
		m_wc.lpszClassName,
		wnd_title,
		dwStyle,
		pos_x,
		pos_y,
		width,
		height,
		NULL,
		NULL,
		NULL,
		this
	);

	if (!m_hWnd) {
		return false;
	}

	if (!popup)
	{
		BOOL value = TRUE;
		::DwmSetWindowAttribute(m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
	}

	if (popup)
	{
		MARGINS margins = { -1 };
		::DwmExtendFrameIntoClientArea(m_hWnd, &margins);
	}

	return true;
}

void CWindow::Release()
{
	if (!m_hWnd)
		return;

	::DestroyWindow(m_hWnd);
	::UnregisterClass(m_wc.lpszClassName, m_wc.hInstance);
}

void CWindow::SetWindowHandle(HWND hWnd)
{
	m_hWnd = hWnd;
}

#ifdef UNICODE
void CWindow::SetWindowTitle(const wchar_t* title)
#else
void CWindow::SetWindowTitle(const char* title)
#endif
{
	::SetWindowText(m_hWnd, title);
}

RECT CWindow::GetWindowRect()
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);

	return rect;
}

HWND CWindow::GetWindowHandle()
{
	return m_hWnd;
}

void CWindow::SetWindowSize(float width, float height)
{
	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	::MoveWindow(m_hWnd, rect.left, rect.top, width, height, TRUE);
}

void CWindow::SetWindowPos(float x, float y)
{
	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	::MoveWindow(m_hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

void CWindow::Show()
{
	::ShowWindow(m_hWnd, SW_SHOW);
	::UpdateWindow(m_hWnd);
}

void CWindow::Hide()
{
	::ShowWindow(m_hWnd, SW_HIDE);
	::UpdateWindow(m_hWnd);
}

void CWindow::Maximize()
{
	::ShowWindow(m_hWnd, SW_MAXIMIZE);
	::UpdateWindow(m_hWnd);
}

void CWindow::Minimize()
{
	::ShowWindow(m_hWnd, SW_MINIMIZE);
	::UpdateWindow(m_hWnd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	CWindow* pWindow = nullptr;

	if (msg == WM_CREATE)
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lparam;
		pWindow = (CWindow*)pCreate->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pWindow);
		pWindow->SetWindowHandle(hwnd);
	}
	else
	{
		pWindow = (CWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (pWindow && pWindow->HandleMessage)
	{
		return pWindow->HandleMessage(hwnd, msg, wparam, lparam);
	}

	// fallback
	return CWindow::DefaultHandler(hwnd, msg, wparam, lparam);
}

LRESULT CWindow::DefaultHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}
