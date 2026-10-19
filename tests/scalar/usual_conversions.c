// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 6: int + unsigned is computed in unsigned (usual arithmetic conversions)
// -1 + 1u is 0 in unsigned arithmetic.
#include <cerastes.h>

unsigned mix_add(int i, unsigned u)
    CERASTES_REQUIRES(i == -1 && u == 1u)
    CERASTES_ENSURES(r, r == 0u)
{
  return i + u;
}
