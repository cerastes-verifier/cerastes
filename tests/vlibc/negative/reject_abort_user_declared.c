// expect: rejected cerastes-call_without_contract @ L15
// mode: c23, c++20
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 6 (and 0015 item 5 as it was): a user declaration of abort, with no <stdlib.h> and no contract, has no contract; the call is rejected, not trusted
// In C++ the declaration has C++ linkage (a look-alike, not the library's); in C a reserved name with no contract written is not rejected either.
#include <cerastes.h>

void abort(void);

int small(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 5) {
    abort();
  }
  return x;
}
