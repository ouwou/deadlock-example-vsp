#pragma once

#include <source2sdk/client/ECurrencySource.hpp>
#include <source2sdk/client/ECurrencyType.hpp>
#include <source2sdk/client/EntitySubclassScope_t.hpp>
#include <source2sdk/client/EStatsType.hpp>

namespace deadlock {
using ECurrencySource = source2sdk::client::ECurrencySource;
using ECurrencyType = source2sdk::client::ECurrencyType;
using EntitySubclassScope_t = source2sdk::client::EntitySubclassScope_t;
using EStatsType = source2sdk::client::EStatsType;

enum HeroID_t : uint32_t {
    Abrams = 6,
    Bebop = 15,
    Dynamo = 11,
    GreyTalon = 17,
    Haze = 13,
    Infernus = 1,
    Ivy = 20,
    Kelvin = 12,
    LadyGeist = 4,
    Lash = 31,
    McGinnis = 8,
    MoAndKrill = 18,
    Paradox = 10,
    Pocket = 50,
    Seven = 2,
    Shiv = 19,
    Vindicta = 3,
    Viscous = 35,
    Warden = 25,
    Wraith = 7,
    Yamato = 27,
};
} // namespace deadlock
