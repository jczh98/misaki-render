#pragma once

#ifdef _MSC_VER
#pragma warning(push, 4)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#pragma warning(disable : 4146)
#pragma warning(disable : 4305)
#pragma warning(disable : 4244)
#pragma warning(disable : 4201)
#pragma warning(disable : 4324)
#pragma warning(disable : 4910)

#else
#pragma GCC diagnostic error "-Wall"
#pragma clang diagnostic error "-Wall"
#pragma GCC diagnostic ignored "-Wc++11-compat"
#pragma clang diagnostic ignored "-Wc++11-compat"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wattributes"
#pragma clang diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#pragma clang diagnostic ignored "-Wparentheses-equality"
#endif