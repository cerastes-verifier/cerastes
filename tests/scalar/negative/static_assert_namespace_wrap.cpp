// expect: error overflow @ L8
// twin: tests/scalar/static_assert_scopes.cpp
// Pins: 0024 item 2: static_assert_scopes.cpp with 65536u * 65536u == 0u inside the namespace
#include <cerastes.h>

namespace limits {
static_assert(sizeof(int) == 4, "int is 32 bits");
static_assert(65536u * 65536u == 0u, "wraps");
}

extern "C" {
static_assert(sizeof(long long) >= 8, "long long");
static_assert(255u * 255u == 65025u, "another product that fits");
}

int four(void)
    CERASTES_ENSURES(r, r == 4)
{
  return (int)sizeof(int);
}
