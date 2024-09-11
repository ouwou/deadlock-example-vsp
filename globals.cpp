#include "globals.hpp"

CreateInterfaceFn g_pfnServerCreateInterface = nullptr;
CreateInterfaceFn g_pfnEngineCreateInterface = nullptr;

HMODULE g_hServerModule = nullptr;
AppSystemConnectFn g_pfnServerConfigConnect = nullptr;

namespace interfaces {
CGameResourceService *g_pGameResourceService = nullptr;
ICvar *g_pCvar = nullptr;
}; // namespace interfaces

CGameEntitySystem *GameEntitySystem() {
    return interfaces::g_pGameResourceService->GetGameEntitySystem();
}
