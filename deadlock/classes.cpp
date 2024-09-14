#include "classes.hpp"

// maybe there is a better name for this
source2sdk::client::CEntitySubclassVDataBase *deadlock::GetVDataInstanceByName(EntitySubclassScope_t type, const char *name) {
    using GetVDataInstanceByNameFn = source2sdk::client::CEntitySubclassVDataBase *(__fastcall *)(EntitySubclassScope_t, const char *);
    static const auto GetVDataInstanceByNameAddr = sigscan::scan(g_hServerModule, "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xFA\x8B\xF1", "xxxx?xxxx?xxxxxxxxxx");
    return reinterpret_cast<GetVDataInstanceByNameFn>(GetVDataInstanceByNameAddr)(type, name);
}

/*
deadlock::CitadelHeroData_t *deadlock::CCitadelHeroVDataManager::GetHeroDataFromName(const char *name) {
    static const auto addr = sigscan::scan(g_hServerModule, "\x40\x53\x48\x83\xEC\x20\x49\x8B\xC0\x48\x8D\x0D", "xxxxxxxxxxxx");
    static const auto sub_displacement = *reinterpret_cast<int32_t *>(addr + 26);
    static const auto subaddr = addr + sub_displacement + 30;
    ConMsg("subaddr=%llx\n", subaddr);

    using GetHeroIDFromNameFn = deadlock::CitadelHeroData_t *(__fastcall *)(CCitadelHeroVDataManager *, const char *, void *);
    return reinterpret_cast<GetHeroIDFromNameFn>(subaddr)(this, name, nullptr);
}
*/

void deadlock::CModifierProperty::AddModifier(source2sdk::server::CBaseEntity *entity, source2sdk::server::CCitadelModifierVData *vdata, KeyValues3 *kv3) {
    using AddModifierFn = void(__fastcall *)(CModifierProperty *, source2sdk::server::CBaseEntity *, uint32_t, int32_t, source2sdk::server::CCitadelModifierVData *, KeyValues3 *, void *);
    static const auto AddModifierAddr = sigscan::scan(g_hServerModule, "\x4C\x8B\xDC\x49\x89\x53\x00\x53\x56", "xxxxxx?xx");
    reinterpret_cast<AddModifierFn>(AddModifierAddr)(this, entity, 0xFFFFFFFF, 0, vdata, kv3, nullptr);
}

void deadlock::CCitadelPlayerPawn::GiveCurrency(deadlock::ECurrencyType currency_type, int amount) {
    using GiveCurrencyFn = void(__fastcall *)(CCitadelPlayerPawn *, ECurrencyType, int, ECurrencySource, unsigned a5, int a6, int a7, void *a8, void *a9);
    static const auto GiveCurrencyAddr = sigscan::scan(g_hServerModule, "\x45\x85\xC0\x0F\x84\x00\x00\x00\x00\x55", "xxxxx????x");
    reinterpret_cast<GiveCurrencyFn>(GiveCurrencyAddr)(this, currency_type, amount, ECurrencySource::EStartingAmount, 0, 0, 0, 0, 0);
}

void deadlock::CCitadelPlayerPawn::GiveItem(const char *item_name) {
    using GiveItemFn = void(__fastcall *)(CCitadelPlayerPawn *, const char *);
    static const auto GiveItemAddr = sigscan::scan(g_hServerModule, "\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x30\x48\x8D\x99", "xxxx?xxxx?xxxxxxxxx");
    reinterpret_cast<GiveItemFn>(GiveItemAddr)(this, item_name);
}

void deadlock::CCitadelPlayerPawn::AddModifier(const char *name, float duration) {
    KeyValues3 kv3;
    if (duration > 0.f) {
        kv3.FindOrCreateMember("duration")->SetFloat(duration);
    }
    auto *vdata = reinterpret_cast<source2sdk::server::CCitadelModifierVData *>(GetVDataInstanceByName(EntitySubclassScope_t::SUBCLASS_SCOPE_MODIFIERS, name));
    reinterpret_cast<CModifierProperty *>(m_pModifierProp)->AddModifier(this, vdata, &kv3);
}

deadlock::HeroID_t deadlock::CCitadelPlayerPawn::GetHeroID() {
    return static_cast<HeroID_t>(m_CCitadelHeroComponent.m_nHeroID.m_Value);
}

void deadlock::CCitadelPlayerPawn::ChangeHero(deadlock::CitadelHeroData_t *hero) {
    using ChangeHeroFn = void(__fastcall *)(CCitadelPlayerPawn *, CitadelHeroData_t * hero);
    static const auto ChangeHeroAddr = sigscan::scan(g_hServerModule, "\x48\x85\xD2\x0F\x84\x00\x00\x00\x00\x48\x89\x54\x24", "xxxxx????xxxx");
    reinterpret_cast<ChangeHeroFn>(ChangeHeroAddr)(this, hero);
}

deadlock::CCitadelPlayerPawn *deadlock::CCitadelPlayerController::GetPawn() {
    auto handle = *reinterpret_cast<CHandle<CCitadelPlayerPawn> *>(m_hHeroPawn);
    return handle.Get();
}

deadlock::CitadelHeroData_t *deadlock::CCitadelHeroManager::GetHeroDataFromID(HeroID_t hero_id) {
    using GetHeroDataFromIDFn = deadlock::CitadelHeroData_t *(__fastcall *)(deadlock::CCitadelHeroManager *, HeroID_t hero_id);
    static const auto GetHeroDataFromIDAddr = sigscan::scan(g_hServerModule, "\x48\x8B\xC1\x85\xD2", "xxxxx");
    return reinterpret_cast<GetHeroDataFromIDFn>(GetHeroDataFromIDAddr)(this, hero_id);
}

deadlock::CitadelHeroData_t *deadlock::CCitadelHeroManager::GetHeroDataFromName(const char *hero_name) {
    return GetHeroDataFromID(GetHeroIDFromName(hero_name));
}

deadlock::HeroID_t deadlock::CCitadelHeroManager::GetHeroIDFromName(const char *hero_name) {
    using GetHeroDataFromNameFn = deadlock::HeroID_t *(__fastcall *)(deadlock::CCitadelHeroManager *, deadlock::HeroID_t *, const char *);
    static const auto GetHeroDataFromNameAddr = sigscan::scan(g_hServerModule, "\x40\x53\x48\x83\xEC\x20\x49\x8B\xC0\x48\x8D\x0D", "xxxxxxxxxxxx");
    ConMsg("GetHeroDataFromNameAddr=%llx\n", GetHeroDataFromNameAddr);
    HeroID_t out;
    reinterpret_cast<GetHeroDataFromNameFn>(GetHeroDataFromNameAddr)(this, &out, hero_name);
    return out;
}
