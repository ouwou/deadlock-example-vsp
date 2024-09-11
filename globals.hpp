#pragma once

#include <interface.h>
#include <windef.h>

class IAppSystem;
class ICvar;
class CGameEntitySystem;

class CGameResourceService {
public:
    CGameEntitySystem* GetGameEntitySystem() {
        return *reinterpret_cast<CGameEntitySystem **>(reinterpret_cast<uintptr_t>(this) + 88);
    }
};

using AppSystemConnectFn = bool (*)(IAppSystem *pAppSystem, CreateInterfaceFn fnCreateInterface);

extern CreateInterfaceFn g_pfnServerCreateInterface;
extern CreateInterfaceFn g_pfnEngineCreateInterface;

extern HMODULE g_hServerModule;
extern AppSystemConnectFn g_pfnServerConfigConnect;

namespace interfaces {
extern CGameResourceService *g_pGameResourceService;
extern ICvar *g_pCvar;
};

// required by entityidentity.cpp
CGameEntitySystem *GameEntitySystem();
