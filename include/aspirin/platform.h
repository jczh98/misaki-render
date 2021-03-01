#pragma once

namespace aspirin {

#if defined(_MSC_VER)
#define APR_EXPORT __declspec(dllexport)
#define APR_IMPORT __declspec(dllimport)
#define APR_NOINLINE __declspec(noinline)
#define APR_INLINE __forceinline
#else
#define APR_EXPORT __attribute__((visibility("default")))
#define APR_IMPORT
#define APR_NOINLINE __attribute__((noinline))
#define APR_INLINE __attribute__((always_inline)) inline
#endif

} // namespace aspirin