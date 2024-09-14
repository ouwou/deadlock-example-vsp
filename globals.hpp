#pragma once

#include <cstdint>
#include <interface.h>
#include <windef.h>

class IAppSystem;
class ICvar;
class IGameEventManager2;
class ISource2GameClients;
class ISource2Server;
class CGameEntitySystem;
class CGameResourceService;

namespace deadlock {
class CCitadelHeroVDataManager;
class CCitadelHeroManager;
} // namespace deadlock

using AppSystemConnectFn = bool (*)(IAppSystem *pAppSystem, CreateInterfaceFn fnCreateInterface);
using GameEventManager_InitFn = void (*)(IGameEventManager2 *);
extern GameEventManager_InitFn g_pfnGameEventManager_InitOrig;

class CBaseEntity;
class Vector;
struct CTakeDamageInfo;
using CTakeDamageInfo_CtorFn = CTakeDamageInfo *(*)(CTakeDamageInfo *thisptr,
                                                    CBaseEntity *inflictor,
                                                    CBaseEntity *attacker,
                                                    CBaseEntity *ability,
                                                    Vector *vecDamageForce,
                                                    Vector *vecDamagePosition,
                                                    Vector *vecReportedPosition,
                                                    float flDamage,
                                                    int32_t bitsDamageType,
                                                    int32_t iDamageCustom);
extern CTakeDamageInfo_CtorFn g_pfnCTakeDamageInfo_CtorOrig;

extern CreateInterfaceFn g_pfnServerCreateInterface;
extern CreateInterfaceFn g_pfnEngineCreateInterface;

extern HMODULE g_hServerModule;
extern AppSystemConnectFn g_pfnServerConfigConnect;

namespace interfaces {
extern deadlock::CCitadelHeroManager *g_pCitadelHeroManager;
extern deadlock::CCitadelHeroVDataManager *g_pCitadelHeroVDataManager;
extern CGameResourceService *g_pGameResourceService;
extern IGameEventManager2 *g_pGameEventManager2;
extern ISource2GameClients *g_pSource2GameClients;
extern ISource2Server *g_pSource2Server;
extern ICvar *&g_pCvar;
}; // namespace interfaces

// defined in global scope for convar.cpp
extern ICvar *g_pCVar;

// required by entityidentity.cpp
CGameEntitySystem *GameEntitySystem();
