#pragma once

namespace nekoba {

#if defined(_MSC_VER)
#  define NEK_EXPORT   __declspec(dllexport)
#  define NEK_IMPORT   __declspec(dllimport)
#  define NEK_NOINLINE __declspec(noinline)
#  define NEK_INLINE   __forceinline
#else
#  define NEK_EXPORT    __attribute__ ((visibility("default")))
#  define NEK_IMPORT
#  define NEK_NOINLINE  __attribute__ ((noinline))
#  define NEK_INLINE    __attribute__((always_inline)) inline
#endif

}