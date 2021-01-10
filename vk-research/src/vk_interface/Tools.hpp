#pragma once

#include <iostream>
#include <cstdint>

#define VK_FLAGS_NONE 0

#define VK_ASSERT(result)                           \
{                                                   \
    VkResult r = result;                            \
    if (r != VK_SUCCESS) {                     \
        std::cerr << "Fatal: VkResult is " << static_cast<int>(r) << std::endl << "FILE: " << __FILE__ << ", line: " << __LINE__ << std::endl; \
        assert(r == VK_SUCCESS);                    \
    }                                               \
}

#if defined(DEBUG) || defined(_DEBUG)   
#define DEBUG_ONLY(exp) exp
#define RELEASE_ONLY(exp)
#define DEBUG_OR_RELEASE(deb, rel) deb
#else                                   
#define DEBUG_ONLY(exp)
#define RELEASE_ONLY(exp) exp
#define DEBUG_OR_RELEASE(deb, rel) rel
#endif

#define STRING_LITERAL(value) #value

template<typename T>
inline bool IsPowerOf2(T value)
{
    return (value & (value - 1)) == 0;
}


template<typename T, typename U>
inline T RoundToMultipleOfPOT(T value, U pot)
{
    return (value + static_cast<T>(pot - 1)) & ~(static_cast<T>(pot - 1));
}



template<typename TBitFormat, typename T>
inline T BitwiseEnumAND(T lhs, T rhs)
{
    return static_cast<T>(static_cast<TBitFormat>(lhs) & static_cast<TBitFormat>(rhs));
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



template<typename TBitFormat, typename T>
inline T BitwiseEnumOR(T lhs, T rhs)
{
    return static_cast<T>(static_cast<TBitFormat>(lhs) | static_cast<TBitFormat>(rhs));
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

// VmaCountBitsSet
// Returns number of bits set to 1 in (v).
static inline uint32_t ToolCountBitsSet(std::uint32_t v)
{
    std::uint32_t c = v - ((v >> 1) & 0x55555555);
    c = ((c >> 2) & 0x33333333) + (c & 0x33333333);
    c = ((c >> 4) + c) & 0x0F0F0F0F;
    c = ((c >> 8) + c) & 0x00FF00FF;
    c = ((c >> 16) + c) & 0x0000FFFF;
    return c;
}

std::uint32_t constexpr TOOL_INVALID_ID = std::numeric_limits<std::uint32_t>::max();

#define ToolSetMemZero(mem) std::memset(&mem, 0, sizeof(mem));
#define ToolSetMemZeroArray(mem) std::memset(mem, 0, sizeof(mem));

#define ToolCopyMemory(dest, src) std::memcpy(&dest, &src, sizeof(src));
#define ToolCopyMemoryArray(dest, src) std::memcpy(dest, src, sizeof(src));
