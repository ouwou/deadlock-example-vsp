#pragma once

#include "defs.hpp"
#include "enums.hpp"

#include "../sigscan.hpp"

#include <source2sdk/client/ECurrencySource.hpp>
#include <source2sdk/client/ECurrencyType.hpp>
#include <source2sdk/server/CCitadelPlayerController.hpp>
#include <source2sdk/server/CCitadelPlayerPawn.hpp>

template<typename T>
T vfunc(void *vft, int i) {
    return (*reinterpret_cast<T **>(vft))[i];
}

namespace deadlock {
struct CCitadelPlayerPawn : public source2sdk::server::CCitadelPlayerPawn {
    void GiveCurrency(deadlock::ECurrencyType currency_type, int amount) {
        using GiveCurrencyFn = void(__fastcall *)(CCitadelPlayerPawn *, ECurrencyType, int, ECurrencySource, unsigned a5, int a6, int a7, void *a8, void *a9);
        static const auto GiveCurrencyAddr = sigscan::scan(g_hServerModule, "\x45\x85\xC0\x0F\x84\x00\x00\x00\x00\x55", "xxxxx????x");
        reinterpret_cast<GiveCurrencyFn>(GiveCurrencyAddr)(this, currency_type, amount, ECurrencySource::EStartingAmount, 0, 0, 0, 0, 0);
    }

    void GiveItem(const char *item_name) {
        using GiveItemFn = void(__fastcall *)(CCitadelPlayerPawn *, const char *);
        static const auto GiveItemAddr = sigscan::scan(g_hServerModule, "\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x30\x48\x8D\x99", "xxxx?xxxx?xxxxxxxxx");
        reinterpret_cast<GiveItemFn>(GiveItemAddr)(this, item_name);
    }
};

struct CCitadelPlayerController : public source2sdk::server::CCitadelPlayerController {
    CCitadelPlayerPawn *GetPawn() {
        auto handle = *reinterpret_cast<CHandle<CCitadelPlayerPawn> *>(m_hHeroPawn);
        return handle.Get();
    }
};
} // namespace deadlock
