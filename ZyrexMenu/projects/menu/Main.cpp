#define APP_NAME        "Vision"
#define APP_VERSION     "2.0.0"
#define APP_BUILD       __DATE__

#include <thread>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <Console.h>
#include <memory/memory.h>
#include <Offsets/OffsetLoader.hpp>
#include <Offsets/AutoUpdater.h>
#include <settings.h>
#include <sdk/sdk.h>
#include <game/game.h>
#include <cache/cache.h>
#include <game/rescan/rescan.h>
#include <features/football/football.h>
#include <features/config/config.h>

#include "Menu.h"
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_dx11.h>

ID3D11Device* g_pD3DDevice = nullptr;
ID3D11DeviceContext* g_pD3DCtx = nullptr;

static bool g_bRunning = true;
static bool g_bMenuVisible = true;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK GlobalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    // Forward non-ImGui input to the Roblox game window so the overlay
    // doesn't freeze gameplay when the menu is visible.
    if (g_bMenuVisible) {
        ImGuiIO& io = ImGui::GetIO();
        switch (msg)
        {
        case WM_KEYDOWN: case WM_KEYUP:
        case WM_SYSKEYDOWN: case WM_SYSKEYUP:
        case WM_CHAR:
            if (!io.WantCaptureKeyboard) {
                HWND ghw = game::get_roblox_window();
                if (ghw) { PostMessage(ghw, msg, wParam, lParam); return 0; }
            }
            break;
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
            if (!io.WantCaptureMouse) {
                HWND ghw = game::get_roblox_window();
                if (ghw) {
                    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                    ScreenToClient(ghw, &pt);
                    SendMessage(ghw, msg, wParam, MAKELPARAM(pt.x, pt.y));
                    return 0;
                }
            }
            break;
        case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
            if (!io.WantCaptureMouse) {
                HWND ghw = game::get_roblox_window();
                if (ghw) {
                    SendMessage(ghw, msg, wParam, lParam);
                    return 0;
                }
            }
            break;
        }
    }

    switch (msg)
    {
    case WM_DESTROY:
    case WM_CLOSE:
        ExitProcess(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int)
{
    try
    {
        Console::Initialize();
        Console::Info("=== %s v%s ===", APP_NAME, APP_VERSION);
        Console::Info("Starting %s v%s...", APP_NAME, APP_VERSION);

        Console::Info("[1/4] Creating overlay window...");

        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(wc);
        wc.style = CS_CLASSDC;
        wc.lpfnWndProc = GlobalWndProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = "VisionOverlay";

        if (!RegisterClassExA(&wc))
            Console::Warn("     (Window class may already exist - continuing)");

        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);

        HWND hWnd = CreateWindowExA(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            "VisionOverlay", APP_NAME,
            WS_POPUP,
            0, 0, sw, sh,
            nullptr, nullptr, hInstance, nullptr
        );

        if (!hWnd)
        {
            Console::Error("[FAILED] CreateWindowEx returned NULL (error: %lu)", GetLastError());
            MessageBoxA(nullptr, "Failed to create overlay window.", "Fatal Error", MB_OK | MB_ICONERROR);
            return 1;
        }

        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
        MARGINS m = { -1 };
        DwmExtendFrameIntoClientArea(hWnd, &m);
        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
        Console::Info("[OK] Overlay: %dx%d", sw, sh);

        Console::Info("[2/4] Creating DirectX 11 device...");

        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferCount = 1;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.Width = 0;
        scd.BufferDesc.Height = 0;
        scd.OutputWindow = hWnd;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        scd.Windowed = TRUE;
        scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        scd.SampleDesc.Count = 1;
        scd.SampleDesc.Quality = 0;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

        IDXGISwapChain* pSwapChain = nullptr;
        ID3D11Device* pDevice = nullptr;
        ID3D11DeviceContext* pContext = nullptr;
        D3D_FEATURE_LEVEL featLevel;
        D3D_FEATURE_LEVEL featLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            featLevels, 2, D3D11_SDK_VERSION, &scd,
            &pSwapChain, &pDevice, &featLevel, &pContext
        );

        if (hr == DXGI_ERROR_UNSUPPORTED)
        {
            Console::Warn("     Hardware unsupported, trying WARP...");
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
                featLevels, 2, D3D11_SDK_VERSION, &scd,
                &pSwapChain, &pDevice, &featLevel, &pContext
            );
        }

        if (FAILED(hr) || !pSwapChain || !pDevice || !pContext)
        {
            Console::Error("[FAILED] D3D11CreateDeviceAndSwapChain: 0x%08lX", hr);
            MessageBoxA(nullptr, "Failed to create DirectX 11 device.", "Fatal Error", MB_OK | MB_ICONERROR);
            return 1;
        }

        ID3D11Texture2D* pBackBuf = nullptr;
        pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuf));
        ID3D11RenderTargetView* pRTView = nullptr;
        if (pBackBuf)
        {
            pDevice->CreateRenderTargetView(pBackBuf, nullptr, &pRTView);
            pBackBuf->Release();
        }

        if (!pRTView)
        {
            Console::Error("[FAILED] Could not create render target");
            MessageBoxA(nullptr, "Failed to create render target.", "Fatal Error", MB_OK | MB_ICONERROR);
            return 1;
        }
        Console::Info("[OK] D3D11 ready");
        g_pD3DDevice = pDevice;
        g_pD3DCtx = pContext;

        Console::Info("[3/4] Setting up menu...");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        ImGui::StyleColorsDark();

        if (!ImGui_ImplWin32_Init(hWnd))
        {
            Console::Error("[FAILED] ImGui_ImplWin32_Init failed");
            MessageBoxA(nullptr, "Failed to initialize ImGui Win32.", "Fatal Error", MB_OK | MB_ICONERROR);
            return 1;
        }
        if (!ImGui_ImplDX11_Init(pDevice, pContext))
        {
            Console::Error("[FAILED] ImGui_ImplDX11_Init failed");
            MessageBoxA(nullptr, "Failed to initialize ImGui DX11.", "Fatal Error", MB_OK | MB_ICONERROR);
            return 1;
        }

        if (!Menu::Initialize(hWnd, pDevice, pContext))
        {
            Console::Error("[FAILED] Menu::Initialize failed");
            MessageBoxA(nullptr, "Failed to initialize menu.", "Fatal Error", MB_OK | MB_ICONERROR);
            return 1;
        }
        Console::Info("[OK] Menu ready - overlay is active");

        g_bMenuVisible = true;

        Console::Info("[4/4] Waiting for RobloxPlayerBeta.exe...");
        Console::Info("     (Open Roblox and join a game)");

        std::thread([]()
        {
            const char* BINARY = "RobloxPlayerBeta.exe";
            while (!memory->find_process_id(BINARY))
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            Console::Info("[+] Roblox PID: %d", memory->get_process_id());

            while (!memory->attach_to_process(BINARY))
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            Console::Info("[+] Attached");

            // Try DLL first (modern Roblox), fall back to EXE
            const char* which_mod = "RobloxPlayerBeta.dll";
            if (!memory->find_module_address(which_mod))
            {
                which_mod = "RobloxPlayerBeta.exe";
                memory->find_module_address(which_mod);
            }
            while (!memory->get_module_address())
            {
                which_mod = "RobloxPlayerBeta.dll";
                if (!memory->find_module_address(which_mod))
                {
                    which_mod = "RobloxPlayerBeta.exe";
                    memory->find_module_address(which_mod);
                }
                Console::Debug("[DEBUG] module '%s' not found yet...", which_mod);
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
            Console::Info("[+] Base: 0x%llX (%s)", memory->get_module_address(), which_mod);

            Console::Info("[*] Auto-updating offsets...");
            AutoUpdater::update_all();

            Console::Debug("Offsets: FDM.Pointer=0x%llX  FDM.RealDataModel=0x%llX", Offsets::FakeDataModel::Pointer, Offsets::FakeDataModel::RealDataModel);
            Console::Debug("Offsets: VE.Pointer=0x%llX  VE.FakeDataModel=0x%llX", Offsets::VisualEngine::Pointer, Offsets::VisualEngine::FakeDataModel);

            // ── Sanity: read PE header at base+0 to verify RPM works ──
            uint64_t base_dll = memory->get_module_address();
            uint16_t mz = memory->read<uint16_t>(base_dll);
            Console::Debug("read(base_dll+0x0) = 0x%04X  (expect 0x5A4D)", mz);
            // Read SizeOfImage from PE header
            uint32_t e_lfanew = memory->read<uint32_t>(base_dll + 0x3C);
            uint32_t size_of_image = memory->read<uint32_t>(base_dll + e_lfanew + 0x50);
            Console::Debug("DLL SizeOfImage = 0x%X bytes (%u MB)", size_of_image, size_of_image / (1024*1024));

            // Also find EXE base
            uint64_t base_exe = 0;
            if (memory->find_module_address("RobloxPlayerBeta.exe"))
                base_exe = memory->get_module_address();
            // Offsets from auto-updater are EXE-relative; switch primary to EXE
            if (base_exe) {
                uint64_t fdm_exe_test = memory->read<uint64_t>(base_exe + Offsets::FakeDataModel::Pointer);
                if (fdm_exe_test) {
                    Console::Debug("Offsets are EXE-relative (FDM from EXE = 0x%llX), switching primary", fdm_exe_test);
                    memory->find_module_address("RobloxPlayerBeta.exe");
                }
            }
            if (!base_exe || memory->get_module_address() != base_exe)
                Console::Debug("Keeping DLL as primary base");
            Console::Debug("DLL base = 0x%llX, EXE base = 0x%llX", base_dll, base_exe);

            // Try FDM from both bases
            for (int bi = 0; bi < 2; bi++) {
                uint64_t b = (bi == 0) ? base_dll : base_exe;
                if (b == 0) continue;
                const char* label = (bi == 0) ? "DLL" : "EXE";
                uint64_t fdmp = Offsets::FakeDataModel::Pointer;
                uint64_t val = memory->read<uint64_t>(b + fdmp);
                Console::Debug("%s base + FDM(0x%llX) = 0x%llX", label, fdmp, val);
                uint64_t vep = Offsets::VisualEngine::Pointer;
                uint64_t vev = memory->read<uint64_t>(b + vep);
                Console::Debug("%s base + VE(0x%llX) = 0x%llX", label, vep, vev);
            }

            // Broader scan: every 0x1000 bytes across the image, print non-zero QWORDs
            Console::Debug("Scanning DLL for non-zero QWORDs (every 0x1000)...");
            uint64_t max_scan = (size_of_image < 0x8000000) ? size_of_image : 0x8000000; // max 128MB
            uint64_t nonzero_count = 0;
            for (uint64_t off = 0; off < max_scan; off += 0x1000) {
                uint64_t val = memory->read<uint64_t>(base_dll + off);
                if (val != 0) {
                    Console::Debug("  [base+0x%06llX] = 0x%llX", off, val);
                    nonzero_count++;
                    if (nonzero_count >= 30) {
                        Console::Debug("  ... (stopping after 30 hits)");
                        break;
                    }
                }
            }
            Console::Debug("Scan complete (nonzero_count=%llu)", nonzero_count);

            Console::Info("[*] Loading game...");
            Console::Debug("     FDM offset: 0x%llX, VE offset: 0x%llX", Offsets::FakeDataModel::Pointer, Offsets::VisualEngine::Pointer);
            int sec = 0;
            while (true)
            {
                // Try all read combinations each iteration
                bool found = false;
                uint64_t bases[2] = { base_dll, base_exe };
                const char* base_names[2] = { "DLL", "EXE" };

                for (int bi = 0; bi < 2; bi++) {
                    if (bases[bi] == 0) continue;
                    uint64_t b = bases[bi];

                    // Path 1: FDM -> RealDataModel -> Players
                    uint64_t fake_dm = memory->read<uint64_t>(b + Offsets::FakeDataModel::Pointer);
                    if (fake_dm) {
                        uint64_t real_dm = memory->read<uint64_t>(fake_dm + Offsets::FakeDataModel::RealDataModel);
                        if (real_dm) {
                            game::datamodel = rbx::instance_t(real_dm);
                            game::players = game::datamodel.find_first_child_by_class("Players");
                            if (game::players.address) {
                                Console::Debug("FDM path OK via %s base", base_names[bi]);
                                found = true; break;
                            }
                        }
                    }

                    // Path 2: VE -> FDM -> RealDataModel -> Players
                    uint64_t ve = memory->read<uint64_t>(b + Offsets::VisualEngine::Pointer);
                    if (ve) {
                        uint64_t fdm2 = memory->read<uint64_t>(ve + Offsets::VisualEngine::FakeDataModel);
                        if (fdm2) {
                            uint64_t real_dm = memory->read<uint64_t>(fdm2 + Offsets::FakeDataModel::RealDataModel);
                            if (real_dm) {
                                game::datamodel = rbx::instance_t(real_dm);
                                game::players = game::datamodel.find_first_child_by_class("Players");
                                if (game::players.address) {
                                    Console::Debug("VE path OK via %s base", base_names[bi]);
                                    found = true; break;
                                }
                            }
                        }
                    }
                }

                if (found) {
                    Console::Debug("All lookups OK, breaking loop");
                    break;
                }

                Console::Debug("Loop state: DLL FDM=0x%llX VE=0x%llX  EXE FDM=0x%llX VE=0x%llX  sec=%d",
                    memory->read<uint64_t>(base_dll + Offsets::FakeDataModel::Pointer),
                    memory->read<uint64_t>(base_dll + Offsets::VisualEngine::Pointer),
                    base_exe ? memory->read<uint64_t>(base_exe + Offsets::FakeDataModel::Pointer) : 0,
                    base_exe ? memory->read<uint64_t>(base_exe + Offsets::VisualEngine::Pointer) : 0,
                    sec);

                sec++; if (sec % 4 == 0) Console::Info("     Waiting... (%d sec)", sec / 4);
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
            game::visengine = { memory->read<uint64_t>(memory->get_module_address() + Offsets::VisualEngine::Pointer) };
            Console::Debug("visengine = 0x%llX", game::visengine.address);
            game::get_roblox_window();
            Console::Info("[+] GAME LOADED!");

            Console::Info("[*] Starting feature threads...");
            std::thread(cache::run).detach();
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            std::thread(AutoRescanHandler).detach();

            Console::Info("");
            Console::Info("==================================");
            Console::Info("  SYSTEMS OPERATIONAL!");
            Console::Info("  Press F1 or INSERT for menu");
            Console::Info("==================================");
        }).detach();

        Console::Info("Main loop running - overlay active");

        while (g_bRunning)
        {
            // Frame limiter: controls max framerate
            static ULONGLONG last_frame = 0;
            ULONGLONG now_ms = GetTickCount64();
            int limit_ms = settings::menu::frame_limiter_ms;
            if (limit_ms > 0 && now_ms - last_frame < (ULONGLONG)limit_ms) { Sleep(1); continue; }
            last_frame = now_ms;

            MSG msg;
            while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if ((GetAsyncKeyState(settings::menu::menu_keybind) & 1) || (GetAsyncKeyState(VK_F1) & 1))
            {
                g_bMenuVisible = !g_bMenuVisible;
                Menu::m_bMenuVisible = g_bMenuVisible;
                LONG style = WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
                if (!g_bMenuVisible)
                    style |= WS_EX_TRANSPARENT;
                SetWindowLong(hWnd, GWL_EXSTYLE, style);
            }

            // Feed mouse position manually so ImGui hover/WantCaptureMouse
            // stays correct even when WS_EX_TRANSPARENT blocks mouse events.
            if (g_bMenuVisible) {
                POINT pt;
                GetCursorPos(&pt);
                ImGui::GetIO().MousePos = ImVec2((float)pt.x, (float)pt.y);
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            static bool last_streamproof = false;
            if (settings::menu::streamproof != last_streamproof) {
                last_streamproof = settings::menu::streamproof;
                DWORD aff = settings::menu::streamproof ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE;
                SetWindowDisplayAffinity(hWnd, aff);
                SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }

            // Check if the Roblox window still exists. If the window closed
            // (process went headless), detach + clear state immediately.
            if (game::datamodel.address != 0 && !FindWindowA(nullptr, "Roblox")) {
                game::clear_state();
                memory->detach();
                memory->reset_error_flags();
            }

            {
                static bool printed_waiting = false;
                if (game::datamodel.address != 0) {
                    printed_waiting = false;
                    football::tick();
                } else if (!printed_waiting) {
                    printed_waiting = true;
                    Console::Info("[4/4] Waiting for RobloxPlayerBeta.exe...");
                    Console::Info("     (Open Roblox and join a game)");
                }
            }

            if (g_bMenuVisible)
                Menu::DrawMenu();

            // Detect ImGui X button close: m_bMenuVisible went false without keybind
            if (g_bMenuVisible && !Menu::m_bMenuVisible)
                ExitProcess(0);

            // Dynamically toggle WS_EX_TRANSPARENT so clicks pass through
            // when not hovering over any ImGui widget.
            if (g_bMenuVisible) {
                bool capture = ImGui::GetIO().WantCaptureMouse;
                LONG style = GetWindowLong(hWnd, GWL_EXSTYLE);
                bool has_transparent = (style & WS_EX_TRANSPARENT) != 0;
                if (capture == has_transparent) {  // state mismatch
                    if (capture)
                        style &= ~WS_EX_TRANSPARENT;
                    else
                        style |= WS_EX_TRANSPARENT;
                    SetWindowLong(hWnd, GWL_EXSTYLE, style);
                }
            }

            football::render_status();

            ImGui::Render();
            pContext->OMSetRenderTargets(1, &pRTView, nullptr);
            float clear[4] = { 0,0,0,0 };
            pContext->ClearRenderTargetView(pRTView, clear);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            pSwapChain->Present(settings::menu::vsync ? 1 : 0, 0);
        }

        Menu::Shutdown();
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (pRTView) pRTView->Release();
        if (pSwapChain) pSwapChain->Release();
        if (pContext) pContext->Release();
        if (pDevice) pDevice->Release();

        Console::Info("Program exiting.");
    }
    catch (const std::exception& e)
    {
        Console::Error("[CRASH] Exception: %s", e.what());
        MessageBoxA(nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
    }
    catch (...)
    {
        Console::Error("[CRASH] Unknown exception!");
        MessageBoxA(nullptr, "An unknown exception occurred.", "Fatal Error", MB_OK | MB_ICONERROR);
    }

    return 0;
}
