#pragma once

#include <D3D11.h>

class Menu
{
public:
	static bool Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	static void Shutdown();

public:
	static bool HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	static void DrawMenu();
	static void DrawWatermark();
	inline static bool m_bMenuVisible = true;

private:
	inline static bool m_bInitialized = false;
};
