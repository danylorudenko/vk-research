#pragma once

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <limits>
#include <cassert>
#include <utility>
#include <cstdlib>
#include <Windows.h>

namespace DRE
{

using U8        = std::uint8_t;
using U16       = std::uint16_t;
using U32       = std::uint32_t;
using U64       = std::uint64_t;

using S8        = std::int8_t;
using S16       = std::int16_t;
using S32       = std::int32_t;
using S64       = std::int64_t;

using SizeT     = std::size_t;

using Char      = S8;
using WChar     = S16;

using PtrDiff   = std::ptrdiff_t;




#define DRE_BEGIN_NAMESPACE namespace DRE {
#define DRE_END_NAMESPACE }




#define DRE_MOVE(value) std::move((value))
#define DRE_SWAP(lhs, rhs) std::swap((lhs), (rhs))

#define DRE_UNIMPLEMENTED static_assert(false, "Unimplemented section.")

#define DRE_DEBUG_ARG(arg) , arg



#if defined (DRE_DEBUG)
    #define DRE_LOG(...) { char str[512]; sprintf_s(str, 512, __VA_ARGS__); OutputDebugStringA(str); }
    #define DRE_ASSERT(condition, message) assert((condition) && (message))
    #define DRE_WARNING(condition, message)
    #define DRE_ERROR(condition, message)
#else
    #define DRE_LOG(...)
    #define DRE_ASSERT(condition, message)
    #define DRE_WARNING(condition, message)
    #define DRE_ERROR(condition, message)
#endif // if defined DRE_DEBUG



} // namespace DRE


#define DRE_U8_MAX  (std::numeric_limits<DRE::U8>::max)()
#define DRE_U16_MAX (std::numeric_limits<DRE::U16>::max)()
#define DRE_U32_MAX (std::numeric_limits<DRE::U32>::max)()
#define DRE_U64_MAX (std::numeric_limits<DRE::U64>::max)()

#define DRE_S8_MAX  (std::numeric_limits<DRE::S8>::max)()
#define DRE_S16_MAX (std::numeric_limits<DRE::S16>::max)()
#define DRE_S32_MAX (std::numeric_limits<DRE::S32>::max)()
#define DRE_S64_MAX (std::numeric_limits<DRE::S64>::max)()

#define DRE_U8_MIN  (std::numeric_limits<DRE::U8>::min)()
#define DRE_U16_MIN (std::numeric_limits<DRE::U16>::min)()
#define DRE_U32_MIN (std::numeric_limits<DRE::U32>::min)()
#define DRE_U64_MIN (std::numeric_limits<DRE::U64>::min)()

#define DRE_S8_MIN  (std::numeric_limits<DRE::S8>::min)()
#define DRE_S16_MIN (std::numeric_limits<DRE::S16>::min)()
#define DRE_S32_MIN (std::numeric_limits<DRE::S32>::min)()
#define DRE_S64_MIN (std::numeric_limits<DRE::S64>::min)()

#define DRE_FLT_EPS std::numeric_limits<float>::epsilon()
#define DRE_DBL_EPS std::numeric_limits<double>::epsilon()

