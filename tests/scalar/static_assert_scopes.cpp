// expect: verified
// expect: native-ok
// Pins: 0024 item 2: namespace and linkage-spec scopes are read from the source sequence list (P43)
#include <cerastes.h>

namespace limits {
static_assert(sizeof(int) == 4, "int is 32 bits");
static_assert(65535u * 65537u == 4294967295u, "an unsigned product that fits");
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
