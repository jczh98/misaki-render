#pragma once

namespace misaki {

#if defined(_MSC_VER)
#define MSK_EXPORT __declspec(dllexport)
#define MSK_IMPORT __declspec(dllimport)
#define MSK_NOINLINE __declspec(noinline)
#define MSK_INLINE __forceinline
#else
#define MSK_EXPORT __attribute__((visibility("default")))
#define MSK_IMPORT
#define MSK_NOINLINE __attribute__((noinline))
#define MSK_INLINE __attribute__((always_inline)) inline
#endif

} // namespace misaki