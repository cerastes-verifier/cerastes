// expect: verified
// expect: native-ok
// Pins: 0024 item 2: a local constexpr is a named constant of its function (0017 item 6) read through the case-value path
#include <cerastes.h>

int square_of_k(void)
    CERASTES_ENSURES(r, r == 7)
{
  constexpr unsigned k = 255u;
  static_assert(k * k == 65025u, "a local constexpr, in range");
  return 7;
}
