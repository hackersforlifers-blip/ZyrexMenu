//============ Copyright ImMagic, All rights reserved ============//
//
// Purpose: 
//
//================================================================//

#include "Direct3D11.h"

#include <directxmath.h>
using namespace DirectX;

CDirect3D11::CDirect3D11() : 
    m_pDevice(nullptr),
    m_pDeviceContext(nullptr),
    m_pSwapChain(nullptr), 
    m_pRenderTargetView(nullptr),
    m_bVSyncEnabled(false), 
    m_hWnd(NULL)
{
}

CDirect3D11::~CDirect3D11()
{
}

bool CDirect3D11::Initialize(bool vsync, HWND hwnd)
{
    m_bVSyncEnabled = vsync;
    m_hWnd = hwnd;

    if (!CreateDevice(hwnd))
    {
        CleanUpDevice();
        return false;
    }

	return true;
}

bool CDirect3D11::CreateDevice(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));

    SwapChainDesc.BufferCount                           = 2;
    SwapChainDesc.BufferDesc.Width                      = 0;
    SwapChainDesc.BufferDesc.Height                     = 0;
    SwapChainDesc.BufferDesc.Format                     = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator      = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator    = 1;
    SwapChainDesc.Flags                                 = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    SwapChainDesc.BufferUsage                           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow                          = hWnd;
    SwapChainDesc.SampleDesc.Count                      = 1;
    SwapChainDesc.SampleDesc.Quality                    = 0;
    SwapChainDesc.Windowed                              = TRUE;
    SwapChainDesc.SwapEffect                            = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL FeatureLevel;
    const D3D_FEATURE_LEVEL FeatureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

    HRESULT hResult = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, FeatureLevelArray, 2, D3D11_SDK_VERSION, &SwapChainDesc, &m_pSwapChain, &m_pDevice, &FeatureLevel, &m_pDeviceContext);
    if (hResult == DXGI_ERROR_UNSUPPORTED)
        hResult = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, FeatureLevelArray, 2, D3D11_SDK_VERSION, &SwapChainDesc, &m_pSwapChain, &m_pDevice, &FeatureLevel, &m_pDeviceContext);
    
    if (hResult != S_OK)
        return false;

    if (!CreateRenderTarget())
        return false;

    return true;
}

void CDirect3D11::CleanUpDevice()
{
    CleanUpRenderTarget();

    if (m_pSwapChain) { 
        m_pSwapChain->Release(); 
        m_pSwapChain = nullptr; 
    }

    if (m_pDeviceContext) 
    { 
        m_pDeviceContext->Release(); 
        m_pDeviceContext = nullptr; 
    }

    if (m_pDevice) 
    { 
        m_pDevice->Release(); 
        m_pDevice = nullptr; 
    }
}

bool CDirect3D11::CreateRenderTarget()
{
    HRESULT hResult;
    ID3D11Texture2D* pBackBufferPtr;

    hResult = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBufferPtr));
    if (FAILED(hResult))
        return false;

    hResult = m_pDevice->CreateRenderTargetView(pBackBufferPtr, nullptr, &m_pRenderTargetView);
    if (FAILED(hResult))
        return false;

    pBackBufferPtr->Release();
    pBackBufferPtr = 0;

    return true;
}

void CDirect3D11::CleanUpRenderTarget()
{
    if (m_pRenderTargetView) 
    { 
        m_pRenderTargetView->Release(); 
        m_pRenderTargetView = nullptr; 
    }
}

void CDirect3D11::Shutdown()
{
    CleanUpDevice();
}


void CDirect3D11::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
}

void CDirect3D11::EndScene()
{
    if (m_bVSyncEnabled)
    {
        m_pSwapChain->Present(1, 0);
    }
    else
    {
        m_pSwapChain->Present(0, 0);
    }
}

void CDirect3D11::SetBuffersSize(UINT width, UINT height)
{
    if (width != 0 && height != 0)
    {
        CleanUpRenderTarget();
        m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        width = 0;
        height = 0;

        CreateRenderTarget();
    }
}