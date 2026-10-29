// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 items 5 and 15: a heap read in a decreases is rejected (the caller's measure is evaluated in the current heap, not the callee's entry heap; re-expected by M4.5b1c2: record 0038 item 8)
#include <cerastes.h>
#include <stddef.h>

int down(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p >= 0 && *p < 100)
    CERASTES_DECREASES(*p)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  if (*p == 0) {
    return 0;
  }
  *p -= 1;
  return down(p);
}
