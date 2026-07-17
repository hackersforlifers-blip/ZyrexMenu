//============ Copyright ImMagic, All rights reserved ============//
//
// Purpose: Extensible application framework
//
//================================================================//

#pragma once

#include <Windows.h>

class CApplication
{
public:
    CApplication();
    virtual ~CApplication();

public:
    virtual bool Initialize();        // Made virtual for overriding
    virtual void Shutdown();          // Made virtual for overriding
    virtual void Run();

protected:
    virtual void Frame();

private:
    bool m_bIsRunning;
};