#pragma once


#ifdef PLAT_WIN32
typedef          __int8     int8;
typedef          __int16    int16;
typedef          __int32    int32;
typedef          __int64    int64;
typedef unsigned __int8     uint8;
typedef unsigned __int16    uint16;
typedef unsigned __int32    uint32;
typedef unsigned __int64    uint64;
#else
typedef int8_t              int8;
typedef int16_t             int16;
typedef int32_t             int32;
typedef int64_t             int64;
typedef uint8_t             uint8;
typedef uint16_t            uint16;
typedef uint32_t            uint32;
typedef uint64_t            uint64;
#endif


#if defined (PLAT_WIN32)
#   define FMT_I64d  "%I64d"
#   define FMT_I64u  "%I64u"
#   define atoi64    _atoi64
#   define strtoui64 _strtoui64
#else
#   define FMT_I64d  "%lld"
#   define FMT_I64u  "%llu"
#   define atoi64    atoll
#   define strtoui64 strtoull
#endif
