#include <string>
#include <fstream>
#include <sstream>

#include <eiface.h>
#include <entity2/entitysystem.h>
#include <icvar.h>
#include <igameevents.h>

#include <sourcehook.h>
#include <sourcehook_impl.h>

#include <funchook.h>

#include "globals.hpp"

#include "sigscan.hpp"
#include "deadlock/deadlock.hpp"
#include <queue>

#pragma comment(lib, "psapi")

SourceHook::Plugin g_PLID = 0x6D656F77;
SourceHook::Impl::CSourceHookImpl g_SHImp;
SourceHook::ISourceHook *g_SHPtr = &g_SHImp;

SH_DECL_HOOK4_void(ISource2GameClients, ClientPutInServer, SH_NOATTRIB, 0, CPlayerSlot, char const *, int, uint64);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);
SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);

CON_COMMAND_F(example, "", FCVAR_GAMEDLL | FCVAR_CLIENT_CAN_EXECUTE) {
    if (context.GetPlayerSlot() == -1) return;

    if (auto *controller = reinterpret_cast<deadlock::CCitadelPlayerController *>(GameEntitySystem()->GetEntityInstance(CEntityIndex(context.GetPlayerSlot().Get() + 1)))) {
        controller->m_iTeamNum = 5;
        if (auto *pawn = controller->GetPawn()) {
            pawn->m_iTeamNum = 5;
        }
    }
}

CON_COMMAND_F(giveme, "", FCVAR_GAMEDLL | FCVAR_CLIENT_CAN_EXECUTE) {
    if (context.GetPlayerSlot() == -1 || args.ArgC() < 2) return;

    if (auto *controller = reinterpret_cast<deadlock::CCitadelPlayerController *>(GameEntitySystem()->GetEntityInstance(CEntityIndex(context.GetPlayerSlot().Get() + 1)))) {
        if (auto *pawn = controller->GetPawn()) {
            pawn->GiveItem(args.Arg(1));
        }
    }
}

CON_COMMAND_F(gimmemodifier, "", FCVAR_GAMEDLL | FCVAR_CLIENT_CAN_EXECUTE) {
    if (context.GetPlayerSlot() == -1 || args.ArgC() < 2) return;

    if (auto *controller = reinterpret_cast<deadlock::CCitadelPlayerController *>(GameEntitySystem()->GetEntityInstance(CEntityIndex(context.GetPlayerSlot().Get() + 1)))) {
        if (auto *pawn = controller->GetPawn()) {
            pawn->AddModifier(args.Arg(1), 5.f);
        }
    }
}

CON_COMMAND_F(myent, "", FCVAR_GAMEDLL | FCVAR_CLIENT_CAN_EXECUTE) {
    if (context.GetPlayerSlot() == -1 || args.ArgC() < 2) return;

    if (auto *controller = reinterpret_cast<deadlock::CCitadelPlayerController *>(GameEntitySystem()->GetEntityInstance(CEntityIndex(context.GetPlayerSlot().Get() + 1)))) {
        ConMsg("controller=%llx\n", controller);
        if (auto *pawn = controller->GetPawn()) {
            ConMsg("pawn=%llx\n", pawn);
        }
    }
}

CON_COMMAND(allents, "") {
    for (uint32_t i = 0; i < 0x1000; i++) {
        auto *ent = GameEntitySystem()->GetEntityInstance(CEntityIndex(i));
        if (ent) {
            ConMsg("%d=%s @ %llx\n", i, ent->GetClassname(), ent);
        }
    }
}

void Hook_ClientPutInServer(CPlayerSlot slot, const char *pszName, int type, uint64 xuid) {
    ConColorMsg(Color(255, 0, 255), "hi from hook - %s joined\n", pszName);
}

// use a queue because calling ChangeHero in the pre-hook for FireEvent will mean the player doesnt die
// cant use post-hook either because the event will have been freed

struct SwapHeroEvent {
    deadlock::CCitadelPlayerPawn *AttackerPawn;
    deadlock::CCitadelPlayerPawn *VictimPawn;
    deadlock::CitadelHeroData_t *AttackerHero;
    deadlock::CitadelHeroData_t *VictimHero;
};
std::queue<SwapHeroEvent> g_swapQueue;

bool Hook_GameEventManager_FireEvent(IGameEvent *event, bool bDontBroadcast) {
    if (!event) RETURN_META_VALUE(MRES_IGNORED, false);

    if (!strcmp(event->GetName(), "player_death")) {
        auto *attacker_controller = reinterpret_cast<deadlock::CCitadelPlayerController *>(event->GetPlayerController("attacker"));
        auto *victim_controller = reinterpret_cast<deadlock::CCitadelPlayerController *>(event->GetPlayerController("userid"));
        if (attacker_controller && victim_controller) {
            auto *attacker_pawn = attacker_controller->GetPawn();
            auto *victim_pawn = victim_controller->GetPawn();
            if (attacker_pawn && victim_pawn) {
                const auto attacker_hero = interfaces::g_pCitadelHeroManager->GetHeroDataFromID(attacker_pawn->GetHeroID());
                const auto victim_hero = interfaces::g_pCitadelHeroManager->GetHeroDataFromID(victim_pawn->GetHeroID());
                g_swapQueue.push({ attacker_pawn, victim_pawn, attacker_hero, victim_hero });
            }
        }
    }

    RETURN_META_VALUE(MRES_IGNORED, true);
}

void Hook_GameFrame(bool, bool, bool) {
    while (!g_swapQueue.empty()) {
        auto e = g_swapQueue.front();
        g_swapQueue.pop();

        e.AttackerPawn->ChangeHero(e.VictimHero);
        e.VictimPawn->ChangeHero(e.AttackerHero);
    }
}

void Hook_GameEventManager_Init(IGameEventManager2 *instance) {
    interfaces::g_pGameEventManager2 = instance;
    g_pfnGameEventManager_InitOrig(instance);

    SH_ADD_HOOK(IGameEventManager2, FireEvent, instance, Hook_GameEventManager_FireEvent, false);
}

CTakeDamageInfo *Hook_CTakeDamageInfo_Ctor(CTakeDamageInfo *thisptr,
                                           CBaseEntity *inflictor,
                                           CBaseEntity *attacker,
                                           CBaseEntity *ability,
                                           Vector *vecDamageForce,
                                           Vector *vecDamagePosition,
                                           Vector *vecReportedPosition,
                                           float flDamage,
                                           int32_t bitsDamageType,
                                           int32_t iDamageCustom) {
    ConColorMsg(Color(255, 0, 0), "CTakeDamageInfo = %llx (%s:%s)\n", thisptr, inflictor ? ((CEntityInstance *)inflictor)->GetClassname() : "", attacker ? ((CEntityInstance *)attacker)->GetClassname() : "");
    g_pfnCTakeDamageInfo_CtorOrig(thisptr, inflictor, attacker, ability, vecDamageForce, vecDamagePosition, vecReportedPosition, flDamage, bitsDamageType, iDamageCustom);
    auto *dmg = reinterpret_cast<deadlock::CTakeDamageInfo *>(thisptr);
    ConMsg("dmg=%llx\n", dmg);
    return thisptr;
}

// https://github.com/saul/cvar-unhide-s2/blob/7f72371553a32dde55583c71d581094517fd61c5/main.cpp
// https://github.com/akiver/cs-demo-manager/blob/58b5f9927e8efabe85b916e1da70966a1c450dc1/cs2-server-plugin/cs2-server-plugin/main.cpp#L396

class Listener : public IEntityListener {
    void OnEntityCreated(CEntityInstance *pEntity) override {
        // ConMsg("CREATED: %s\n", pEntity->GetClassname());
    }

    void OnEntitySpawned(CEntityInstance *pEntity) override {
        // ConMsg("SPAWNED: %s\n", pEntity->GetClassname());
    };

    void OnEntityDeleted(CEntityInstance *pEntity) override {
        // ConMsg("DELETED: %s\n", pEntity->GetClassname());
    };

    void OnEntityParentChanged(CEntityInstance *pEntity, CEntityInstance *pNewParent) override {};
};

CON_COMMAND(bleh, "") {
    Listener *listener = new Listener;
    using TestFn = char(__fastcall *)(void *, void *);
    static const auto TestAddr = sigscan::scan(g_hServerModule, "\x48\x89\x5C\x24\x00\x55\x41\x56\x41\x57\x48\x83\xEC\x20\x4C\x63\xB9\x00\x00\x00\x00\x48\x8D\x99\x00\x00\x00\x00\x45\x33\xC0\x48\x8B\xEA\x4C\x8B\xF1\x45\x85\xFF\x7E\x00\x48\x8B\x03\x41\x8B\xD0\x48\x39\x28\x74\x00\x41\xFF\xC0\x48\xFF\xC2\x48\x83\xC0\x08\x49\x3B\xD7\x7C\x00\x44\x3B\xB9", "xxxx?xxxxxxxxxxxx????xxx????xxxxxxxxxxxxx?xxxxxxxxxx?xxxxxxxxxxxxxx?xxx");
    reinterpret_cast<TestFn>(TestAddr)(GameEntitySystem(), listener);
}

CON_COMMAND(cvar_unhide, "") {
    uint64_t mask = FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY | FCVAR_MISSING3;
    int num_unhidden_concommands = 0;
    int num_unhidden_convars = 0;

    {
        ConCommandHandle hdl;
        auto invalid = interfaces::g_pCvar->GetCommand(hdl);
        int idx = 0;
        while (true) {
            hdl.Set(idx++);
            auto concommand = interfaces::g_pCvar->GetCommand(hdl);
            if (concommand == invalid) break;

            if (concommand->GetFlags() & mask) {
                ConMsg("- %s\n", concommand->GetName());
                concommand->RemoveFlags(mask);
                num_unhidden_concommands++;
            }
        }
    }

    {
        ConVarHandle hdl;
        auto invalid = interfaces::g_pCvar->GetConVar(hdl);
        int idx = 0;
        while (true) {
            hdl.Set(idx++);
            auto convar = interfaces::g_pCvar->GetConVar(hdl);
            if (convar == invalid) break;

            if (convar->flags & mask) {
                ConMsg("- %s\n", convar->m_pszName);
                convar->flags &= ~mask;
                num_unhidden_convars++;
            }
        }
    }

    ConColorMsg(Color(255, 0, 255), "%d concommands unhidden\n", num_unhidden_concommands);
    ConColorMsg(Color(255, 0, 255), "%d convars unhidden\n", num_unhidden_convars);
}

class TestFilter : public CTraceFilter {
    using CTraceFilter::CTraceFilter;

    bool ShouldHitEntity(CEntityInstance *pEntity) override {
        // ConMsg("  - ShouldHitEntity: %llx:%s\n", pEntity, pEntity->GetClassname());
        return true;
    }
};

using TestHookFn = void *(__fastcall *)(void *, void *, CTraceFilter *);
TestHookFn g_pfnTestHook_Orig = nullptr;
void *Hook_TestHook(void *a1, void *a2, CTraceFilter *a3) {
    // TestFilter filter(a3->m_nInteractsWith, a3->m_nCollisionGroup, true);
    // ConColorMsg(Color(255, 0, 255), "TraceFilter=%llx\n", a3);
    // a3->m_bForceHitEverything = true;
    // constexpr uint64_t INTERACTION_LAYER_TEAM2 = 0x080000000;
    // constexpr uint64_t INTERACTION_LAYER_TEAM3 = 0x100000000;
    // a3->m_nInteractsExclude &= ~(INTERACTION_LAYER_TEAM2 | INTERACTION_LAYER_TEAM3);
    auto *ret = g_pfnTestHook_Orig(a1, a2, a3);
    // ConColorMsg(Color(255, 0, 255), "  Post=%llx\n", a3);
    return ret;
}

using MagicFn = void *(__fastcall *)(int a1, void *a2, void *a3, void *a4, void *a5, void *a6, void *a7, void *a8, void *a9, void *a10, void *a11, void *a12, ...);
MagicFn g_pfnMagic_Orig = nullptr;
void *Hook_Magic(int a1, void *a2, void *a3, void *a4, void *a5, void *a6, void *a7, void *a8, void *a9, void *a10, void *a11, void *a12, ...) {
    auto *handle = reinterpret_cast<CHandle<deadlock::CCitadelPlayerPawn> *>(reinterpret_cast<uintptr_t>(a3) + 0x8);
    uint64_t *exclude = reinterpret_cast<uint64_t *>(reinterpret_cast<uintptr_t>(a3) + 0x78);
    constexpr uint64_t Citadel_Team_Amber = 0x080000000;
    constexpr uint64_t Citadel_Team_Sapphire = 0x100000000;
    *exclude &= ~(Citadel_Team_Amber | Citadel_Team_Sapphire);
    *reinterpret_cast<int32_t *>(reinterpret_cast<uintptr_t>(a3) + 0xC) = handle->m_data;

    auto *r = g_pfnMagic_Orig(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    return r;
}

enum InteractionLayer : uint64_t {
    solid = 0x1,
    hitboxes = 0x2,
    trigger = 0x4,
    sky = 0x8,
    playerclip = 0x10,
    npcclip = 0x20,
    blocklos = 0x40,
    blocklight = 0x80,
    ladder = 0x100,
    pickup = 0x200,
    blocksound = 0x400,
    nodraw = 0x800,
    window = 0x1000,
    passbullets = 0x2000,
    worldGeometry = 0x4000,
    water = 0x8000,
    slime = 0x10000,
    touchall = 0x20000,
    player = 0x40000,
    npc = 0x80000,
    debris = 0x100000,
    physics_prop = 0x200000,
    NavIgnore = 0x400000,
    NavLocalIgnore = 0x800000,
    PostProcessingVolume = 0x1000000,
    UnusedLayer3 = 0x2000000,
    CarriedObject = 0x4000000,
    pushaway = 0x8000000,
    serverentityonclient = 0x10000000,
    CarriedWeapon = 0x20000000,
    StaticLevel = 0x40000000,
    Citadel_Team_Amber = 0x80000000,
    Citadel_Team_Sapphire = 0x100000000,
    Citadel_Team_Neutal = 0x200000000,
    Citadel_Ability = 0x400000000,
    Citadel_Bullet = 0x800000000,
    Citadel_Projectile = 0x1000000000,
    Citadel_Unit_Hero = 0x2000000000,
    Citadel_Unit_Creep = 0x4000000000,
    Citadel_Unit_Building = 0x8000000000,
    Citadel_Unit_Prop = 0x10000000000,
    Citadel_Unit_Minion = 0x20000000000,
    Citadel_Unit_Boss = 0x40000000000,
    Citadel_Unit_GoldOrb = 0x80000000000,
    Citadel_Unit_World_Prop = 0x100000000000,
    Citadel_Unit_Trophy = 0x200000000000,
    Citadel_Unit_Zipline = 0x400000000000,
    Citadel_Mantle_Hidden = 0x800000000000,
    Citadel_Obscured = 0x1000000000000,
    Citadel_Time_Warp = 0x2000000000000,
    Citadel_Foliage = 0x4000000000000,
    Citadel_Transparent = 0x8000000000000,
    Citadel_BlockCamera = 0x10000000000000,
    Citadel_Temp_Movement_Blocker = 0x80000000000000,
    Citadel_BlockMantle = 0x100000000000000,
    Citadel_Skyclip = 0x200000000000000,
    Citadel_Valid_Ping_Target = 0x400000000000000,
};

using ExecuteTraceFn = void *(__fastcall *)(void *interface, void *outvar, void *posMaybe, CTraceFilter *filter, int a5, void *a6, char a7);
ExecuteTraceFn g_pfnExecuteTrace_Orig = nullptr;
void *Hook_ExecuteTrace(void *interface, void *outvar, void *posMaybe, CTraceFilter *filter, int a5, void *a6, char a7) {
    bool hasTeamExclusion = (filter->m_nInteractsExclude & (InteractionLayer::Citadel_Team_Amber | InteractionLayer::Citadel_Team_Sapphire)) != 0;
    bool interactsAsWeapon = (filter->m_nInteractsAs & (InteractionLayer::Citadel_Ability | InteractionLayer::Citadel_Bullet)) != 0;

    bool maybePVP = hasTeamExclusion;
    if (maybePVP) {
        filter->m_nInteractsExclude &= ~(InteractionLayer::Citadel_Team_Amber | InteractionLayer::Citadel_Team_Sapphire);
    }
    return g_pfnExecuteTrace_Orig(interface, outvar, posMaybe, filter, a5, a6, a7);
}

bool Connect(IAppSystem *pAppSystem, CreateInterfaceFn fnCreateInterface) {
    auto result = g_pfnServerConfigConnect(pAppSystem, fnCreateInterface);

    g_pfnEngineCreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(GetModuleHandleA("engine2"), "CreateInterface"));

    interfaces::g_pGameResourceService = reinterpret_cast<CGameResourceService *>(g_pfnEngineCreateInterface(GAMERESOURCESERVICESERVER_INTERFACE_VERSION, nullptr));
    if (!interfaces::g_pGameResourceService) {
        Plat_FatalErrorFunc("Could not get GameResourceService");
    }

    interfaces::g_pSource2GameClients = reinterpret_cast<ISource2GameClients *>(g_pfnServerCreateInterface(SOURCE2GAMECLIENTS_INTERFACE_VERSION, nullptr));
    if (!interfaces::g_pSource2GameClients) {
        Plat_FatalErrorFunc("Could not get Source2GameClients");
    }

    interfaces::g_pSource2Server = reinterpret_cast<ISource2Server *>(g_pfnServerCreateInterface(SOURCE2SERVER_INTERFACE_VERSION, nullptr));
    if (!interfaces::g_pSource2Server) {
        Plat_FatalErrorFunc("Could not get Source2Server");
    }

    interfaces::g_pCvar = reinterpret_cast<ICvar *>(fnCreateInterface("VEngineCvar007", nullptr));
    if (!interfaces::g_pCvar) {
        Plat_FatalErrorFunc("Could not get ICvar");
    }

    {
        // todo get all the signatures in one spot this is duplicated
        static const auto addr = sigscan::scan(g_hServerModule, "\x40\x53\x48\x83\xEC\x20\x49\x8B\xC0\x48\x8D\x0D", "xxxxxxxxxxxx");
        static const auto displacement = *reinterpret_cast<int32_t *>(addr + 12);
        interfaces::g_pCitadelHeroVDataManager = reinterpret_cast<deadlock::CCitadelHeroVDataManager *>(addr + displacement + 0x10);
    }

    {
        static const auto addr = sigscan::scan(g_hServerModule, "\x48\x89\x05\x00\x00\x00\x00\x33\xDB", "xxx????xx");
        static const auto displacement = *reinterpret_cast<int32_t *>(addr + 3);
        interfaces::g_pCitadelHeroManager = reinterpret_cast<deadlock::CCitadelHeroManager *>(addr + displacement + 0x7);
        ConMsg("g_pCitadelHeroManager=%llx\n", interfaces::g_pCitadelHeroManager);
    }

    auto hook = funchook_create();
    static const auto GameEventManager_InitAddr = sigscan::scan(g_hServerModule, "\x40\x53\x48\x83\xEC\x20\x48\x8B\x01\x48\x8B\xD9\xFF\x50\x00\x48\x8B\x03", "xxxxxxxxxxxxxx?xxx");
    g_pfnGameEventManager_InitOrig = reinterpret_cast<GameEventManager_InitFn>(GameEventManager_InitAddr);
    funchook_prepare(hook, reinterpret_cast<void **>(&g_pfnGameEventManager_InitOrig), &Hook_GameEventManager_Init);
    funchook_install(hook, 0);

    {
        static const auto CTakeDamageInfo_CtorAddr = sigscan::scan(g_hServerModule, "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x56\x41\x57\x48\x83\xEC\x20\x4D\x8B\xF1", "xxxx?xxxx?xxxx?xxxxxxxxxxxx");
        g_pfnCTakeDamageInfo_CtorOrig = reinterpret_cast<CTakeDamageInfo_CtorFn>(CTakeDamageInfo_CtorAddr);
        ConMsg("g_pfnCTakeDamageInfo_Ctor=%llx\n", g_pfnCTakeDamageInfo_CtorOrig);

        auto hook = funchook_create();
        funchook_prepare(hook, reinterpret_cast<void **>(&g_pfnCTakeDamageInfo_CtorOrig), &Hook_CTakeDamageInfo_Ctor);
        funchook_install(hook, 0);
    }

    {
        static const auto TestHook_Addr = sigscan::scan(g_hServerModule, "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x55\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\xAC\x24\x00\x00\x00\x00\xB8\x20\x63\x00\x00", "xxxx?xxxx?xxxx?xxxxxxxxxxxxx????xxxxx");
        g_pfnTestHook_Orig = reinterpret_cast<TestHookFn>(TestHook_Addr);
        ConMsg("g_pfnTestHook=%llx\n", g_pfnTestHook_Orig);

        // auto hook = funchook_create();
        // funchook_prepare(hook, reinterpret_cast<void **>(&g_pfnTestHook_Orig), &Hook_TestHook);
        // funchook_install(hook, 0);
    }

    {
        static const auto Magic_Addr = sigscan::scan(g_hServerModule, "\x48\x8B\xC4\x4C\x89\x48\x00\x4C\x89\x40\x00\x48\x89\x50\x00\x89\x48", "xxxxxx?xxx?xxx?xx");
        g_pfnMagic_Orig = reinterpret_cast<MagicFn>(Magic_Addr);

        //auto hook = funchook_create();
        //funchook_prepare(hook, reinterpret_cast<void **>(&g_pfnMagic_Orig), &Hook_Magic);
        //funchook_install(hook, 0);
    }

    {
        static const auto ExecuteTrace_Addr = sigscan::scan(g_hServerModule, "\x4C\x89\x44\x24\x00\x55\x53\x56\x41\x55\x41\x56", "xxxx?xxxxxxx");
        g_pfnExecuteTrace_Orig = reinterpret_cast<ExecuteTraceFn>(ExecuteTrace_Addr);

        auto hook = funchook_create();
        funchook_prepare(hook, reinterpret_cast<void **>(&g_pfnExecuteTrace_Orig), &Hook_ExecuteTrace);
        funchook_install(hook, 0);
    }

    SH_ADD_HOOK(ISource2GameClients, ClientPutInServer, interfaces::g_pSource2GameClients, Hook_ClientPutInServer, true);
    SH_ADD_HOOK(IServerGameDLL, GameFrame, interfaces::g_pSource2Server, Hook_GameFrame, true);

    interfaces::g_pCvar->RegisterConCommand(&example_command);
    interfaces::g_pCvar->RegisterConCommand(&giveme_command);
    interfaces::g_pCvar->RegisterConCommand(&gimmemodifier_command);
    interfaces::g_pCvar->RegisterConCommand(&allents_command);
    interfaces::g_pCvar->RegisterConCommand(&myent_command);
    interfaces::g_pCvar->RegisterConCommand(&bleh_command);
    interfaces::g_pCvar->RegisterConCommand(&cvar_unhide_command);

    return result;
}

EXPORT void *CreateInterface(const char *pName, int *pReturnCode) {
    if (!g_pfnServerCreateInterface) {
        std::string game_directory = Plat_GetGameDirectory();
        std::string lib_path = game_directory + "\\citadel\\bin\\win64\\server.dll";
        g_hServerModule = LoadLibraryA(lib_path.c_str());

        if (!g_hServerModule) {
            Plat_FatalErrorFunc("Failed to load server module from %s: %d\n", lib_path.c_str(), GetLastError());
        }

        g_pfnServerCreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(g_hServerModule, "CreateInterface"));
        if (!g_pfnServerCreateInterface) {
            Plat_FatalErrorFunc("Failed to find CreateInterface in server module: %d\n", GetLastError());
        }
    }

    void *original = g_pfnServerCreateInterface(pName, pReturnCode);
    if (!strcmp(pName, "Source2ServerConfig001")) {
        void **vtable = *(void ***)original;

        DWORD dwOldProtect = 0;
        if (!VirtualProtect(vtable, sizeof(void **), PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
            Plat_FatalErrorFunc("VirtualProtect failed: %d\n", GetLastError());
        }

        g_pfnServerConfigConnect = reinterpret_cast<AppSystemConnectFn>(vtable[0]);
        vtable[0] = &Connect;

        if (!VirtualProtect(vtable, sizeof(void **), dwOldProtect, &dwOldProtect)) {
            Plat_FatalErrorFunc("VirtualProtect failed: %d\n", GetLastError());
        }
    }

    return original;
}
