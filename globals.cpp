#include "globals.hpp"

#include "../deadlock/classes.hpp"

CreateInterfaceFn g_pfnServerCreateInterface = nullptr;
CreateInterfaceFn g_pfnEngineCreateInterface = nullptr;

HMODULE g_hServerModule = nullptr;
AppSystemConnectFn g_pfnServerConfigConnect = nullptr;

GameEventManager_InitFn g_pfnGameEventManager_InitOrig = nullptr;
CTakeDamageInfo_CtorFn g_pfnCTakeDamageInfo_CtorOrig = nullptr;

namespace interfaces {
deadlock::CCitadelHeroManager *g_pCitadelHeroManager = nullptr;
deadlock::CCitadelHeroVDataManager *g_pCitadelHeroVDataManager = nullptr;
CGameResourceService *g_pGameResourceService = nullptr;
IGameEventManager2 *g_pGameEventManager2 = nullptr;
ISource2GameClients *g_pSource2GameClients = nullptr;
ISource2Server *g_pSource2Server = nullptr;
ICvar *&g_pCvar = ::g_pCVar;
}; // namespace interfaces

ICvar *g_pCVar = nullptr;

CGameEntitySystem *GameEntitySystem() {
    return interfaces::g_pGameResourceService->GetGameEntitySystem();
}
