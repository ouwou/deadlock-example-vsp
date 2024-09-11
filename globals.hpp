#pragma once

#include <interface.h>
#include <windef.h>

class IAppSystem;
class ICvar;
class ISource2GameClients;
class CGameEntitySystem;

class CGameResourceService {
public:
    CGameEntitySystem *GetGameEntitySystem() {
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
extern ISource2GameClients *g_pSource2GameClients;
extern ICvar *&g_pCvar;
}; // namespace interfaces

// defined in global scope for convar.cpp
extern ICvar *g_pCVar;

// required by entityidentity.cpp
CGameEntitySystem *GameEntitySystem();
