#include <string>

#include <eiface.h>
#include <entity2/entitysystem.h>
#include <icvar.h>

#include <sourcehook.h>
#include <sourcehook_impl.h>

#include "globals.hpp"

#include "sigscan.hpp"
#include "deadlock/deadlock.hpp"

SourceHook::Plugin g_PLID = 0x6D656F77;
SourceHook::Impl::CSourceHookImpl g_SHImp;
SourceHook::ISourceHook *g_SHPtr = &g_SHImp;

SH_DECL_HOOK4_void(ISource2GameClients, ClientPutInServer, SH_NOATTRIB, 0, CPlayerSlot, char const *, int, uint64);

CON_COMMAND_F(example, "", FCVAR_GAMEDLL | FCVAR_CLIENT_CAN_EXECUTE) {
    if (context.GetPlayerSlot() == -1) return;

    if (auto *controller = reinterpret_cast<deadlock::CCitadelPlayerController *>(GameEntitySystem()->GetEntityInstance(CEntityIndex(context.GetPlayerSlot().Get() + 1)))) {
        if (auto *pawn = controller->GetPawn()) {
            pawn->GiveCurrency(deadlock::ECurrencyType::EGold, 100000);
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

void Hook_ClientPutInServer(CPlayerSlot slot, const char *pszName, int type, uint64 xuid) {
    ConColorMsg(Color(255, 0, 255), "hi from hook - %s joined\n", pszName);
}

// https://github.com/saul/cvar-unhide-s2/blob/7f72371553a32dde55583c71d581094517fd61c5/main.cpp
// https://github.com/akiver/cs-demo-manager/blob/58b5f9927e8efabe85b916e1da70966a1c450dc1/cs2-server-plugin/cs2-server-plugin/main.cpp#L396

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

    interfaces::g_pCvar = reinterpret_cast<ICvar *>(fnCreateInterface("VEngineCvar007", nullptr));
    if (!interfaces::g_pCvar) {
        Plat_FatalErrorFunc("Could not get ICvar");
    }

    SH_ADD_HOOK(ISource2GameClients, ClientPutInServer, interfaces::g_pSource2GameClients, Hook_ClientPutInServer, true);

    interfaces::g_pCvar->RegisterConCommand(&example_command);
    interfaces::g_pCvar->RegisterConCommand(&giveme_command);

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
