#pragma once

#include "defs.hpp"
#include "enums.hpp"

#include "../sigscan.hpp"

#include <source2sdk/client/ECurrencySource.hpp>
#include <source2sdk/client/ECurrencyType.hpp>
#include <source2sdk/client/CEntitySubclassVDataBase.hpp>
#include <source2sdk/server/CCitadelModifierVData.hpp>
#include <source2sdk/server/CCitadelPlayerController.hpp>
#include <source2sdk/server/CCitadelPlayerPawn.hpp>
#include <source2sdk/server/CModifierProperty.hpp>

template<typename T>
T vfunc(void *vft, int i) {
    return (*reinterpret_cast<T **>(vft))[i];
}

namespace deadlock {
// maybe there is a better name for this
source2sdk::client::CEntitySubclassVDataBase *GetVDataInstanceByName(EntitySubclassScope_t type, const char *name) {
    using GetVDataInstanceByNameFn = source2sdk::client::CEntitySubclassVDataBase *(__fastcall *)(EntitySubclassScope_t, const char *);
    static const auto GetVDataInstanceByNameAddr = sigscan::scan(g_hServerModule, "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xFA\x8B\xF1", "xxxx?xxxx?xxxxxxxxxx");
    return reinterpret_cast<GetVDataInstanceByNameFn>(GetVDataInstanceByNameAddr)(type, name);
}
} // namespace deadlock

namespace deadlock {
struct CModifierProperty : public source2sdk::server::CModifierProperty {
    void AddModifier(source2sdk::server::CBaseEntity *entity, source2sdk::server::CCitadelModifierVData *vdata, KeyValues3 *kv3) {
        using AddModifierFn = void(__fastcall *)(CModifierProperty *, source2sdk::server::CBaseEntity *, uint32_t, int32_t, source2sdk::server::CCitadelModifierVData *, KeyValues3 *, void *);
        static const auto AddModifierAddr = sigscan::scan(g_hServerModule, "\x4C\x8B\xDC\x49\x89\x53\x00\x53\x56", "xxxxxx?xx");
        reinterpret_cast<AddModifierFn>(AddModifierAddr)(this, entity, 0xFFFFFFFF, 0, vdata, kv3, nullptr);
    }
};

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

    void AddModifier(const char *name, float duration = 0.f) {
        KeyValues3 kv3;
        if (duration > 0.f) {
            kv3.FindOrCreateMember("duration")->SetFloat(duration);
        }
        auto *vdata = reinterpret_cast<source2sdk::server::CCitadelModifierVData *>(GetVDataInstanceByName(EntitySubclassScope_t::SUBCLASS_SCOPE_MODIFIERS, name));
        reinterpret_cast<CModifierProperty *>(m_pModifierProp)->AddModifier(this, vdata, &kv3);
    }
};

struct CCitadelPlayerController : public source2sdk::server::CCitadelPlayerController {
    CCitadelPlayerPawn *GetPawn() {
        auto handle = *reinterpret_cast<CHandle<CCitadelPlayerPawn> *>(m_hHeroPawn);
        return handle.Get();
    }
};
} // namespace deadlock
