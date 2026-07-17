//============ Copyright ImMagic, All rights reserved ============//
//
// Purpose: Extensible application framework
//
//================================================================//

#include "Application.h"
#include "Console.h"

CApplication::CApplication() : m_bIsRunning(false)
{
}

CApplication::~CApplication()
{
}

bool CApplication::Initialize()
{
	Console::Initialize();
    Console::Info("Application Started");

    return true;
}

void CApplication::Shutdown()
{
    m_bIsRunning = false;
}

void CApplication::Run()
{
    m_bIsRunning = true;
    MSG msg;

    while (m_bIsRunning)
    {
        // Process ALL pending messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                m_bIsRunning = false;
        }

        if (!m_bIsRunning)
            break;

        Frame();
    }
}

void CApplication::Frame()
{
}