// expect: error overflow @ L7
// twin: tests/scalar/static_assert_sizeof_size_t.cpp
// Pins: 0024 item 3: `sizeof(std::size_t) == 8 && 65536u * 65536u == 0u` is encoded as a whole
#include <cerastes.h>
#include <cstddef>

static_assert(sizeof(std::size_t) == 8 && 65536u * 65536u == 0u, "wraps");

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}
