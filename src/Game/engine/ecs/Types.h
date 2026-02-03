#ifndef _TYPES_H
#define _TYPES_H

#include <bitset>
#include <cstdint>

namespace Engine
{
    using Entity = uint32_t;
    using ComponentType = uint8_t;
    constexpr Entity MAX_ENTITIES = 4096;
    constexpr ComponentType MAX_COMPONENTS = 32;

    using Signature = std::bitset<MAX_COMPONENTS>;
}

#endif // _TYPES_H