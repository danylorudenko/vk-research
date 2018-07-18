#pragma once

#include <iostream>
#include <cstdint>

#define VK_FLAGS_NONE 0

#define VK_ASSERT(result)                           \
{                                                   \
    VkResult r = result;                            \
    if (result != VK_SUCCESS) {                     \
        std::cerr << "Fatal: VkResult is " << static_cast<int>(r) << std::endl << "FILE: " << __FILE__ << ", line: " << __LINE__ << std::endl; \
        assert(r == VK_SUCCESS);                    \
    }                                               \
}

#define STRING_LITERAL(value) #value



template<typename T, typename U>
inline T RoundToMultipleOfPOT(T value, U pot)
{
    return (value + static_cast<T>(pot - 1)) & ~(static_cast<T>(pot - 1));
}



template<typename T, typename TCastDest>
inline T BitwiseEnumAND(T lhs, T rhs)
{
    return static_cast<TCastDest>(lhs) & static_cast<TCastDest>(rhs);
}

template<typename T>
inline T BitwiseEnumAND32(T lhs, T rhs)
{
    return BitwiseEnumAND<std::uint32_t>(lhs, rhs);
}

template<typename T>
inline T BitwiseEnumAND64(T lhs, T rhs)
{
    return BitwiseEnumAND<std::uint64_t>(lhs, rhs);
}



template<typename T, typename TCastDest>
inline T BitwiseEnumOR(T lhs, T rhs)
{
    return static_cast<TCastDest>(lhs) | static_cast<TCastDest>(rhs);
}

template<typename T>
inline T BitwiseEnumOR32(T lhs, T rhs)
{
    return BitwiseEnumOR<std::uint32_t>(lhs, rhs);
}

template<typename T>
inline T BitwiseEnumOR64(T lhs, T rhs)
{
    return BitwiseEnumAND<std::uint64_t>(lhs, rhs);
}