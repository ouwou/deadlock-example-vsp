#pragma once

#include <cstdint>

#include "../globals.hpp"

using CSoundEventName = uint8_t[0x10];
using CGlobalSymbol = uint8_t[0x08];
using CNetworkedQuantizedFloat = uint8_t[0x08];
using CPanoramaImageName = uint8_t[0x10];

template<typename T>
struct CHandle {
    T *Get() {
        return reinterpret_cast<T *>(GameEntitySystem()->GetEntityInstance(CEntityIndex(m_data & 0x7fff)));
    }

    int32_t m_data;
};
