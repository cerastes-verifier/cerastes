// expect: verified
// expect: native-ok
// Pins: 0046 item 7: a user contract on a platform function vlibc does not cover (abs) is accepted after <stdlib.h>; the platform's own attributes are skipped by position (abs is not a reserved name, so the user's contract is trusted as a system function's is, 0011 item 5; the noexcept helper matches glibc's `__THROW`)
#include <stdlib.h>
#include <cerastes.h>
#include "headers/glibc_noexcept.h"

int abs(int x) VLIBC_TEST_NOEXCEPT
    CERASTES_REQUIRES(x > -1000)
    CERASTES_ENSURES(r, r >= 0 && (r == x || r == -x));

int magnitude(int v)
    CERASTES_REQUIRES(-100 <= v && v <= 100)
    CERASTES_ENSURES(r, r >= 0)
{
  return abs(v);
}
