//============ Copyright ImMagic, All rights reserved ============//
//
// Purpose: 
//
//================================================================//

#pragma once

#include <d3d11.h>
// #include <d3dx11.h> - REMOVED: not needed by this header; <d3d11.h> provides all required types

class CDirect3D11
{
public:
	CDirect3D11();
	~CDirect3D11();

public:
	bool Initialize(bool vsync, HWND hwnd);
	void Shutdown();

public:
	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();

public:
	bool CreateDevice(HWND hwnd);
	void CleanUpDevice();
	bool CreateRenderTarget();
	void CleanUpRenderTarget();
	void SetBuffersSize(UINT width, UINT height);

public:
	ID3D11Device* GetDevice()				{ return m_pDevice; };
	ID3D11DeviceContext* GetDeviceContext() { return m_pDeviceContext; };
	HWND GetWindow()						{ return m_hWnd; };

private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;
//	bool m_bSwapChainOccluded;
	HWND m_hWnd;

private:
	bool m_bVSyncEnabled;
};
