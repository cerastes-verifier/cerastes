// expect: rejected cerastes-uninit_ref_initialized @ L16
// mode: c23, c++20
// twin: tests/memory/init_strict_ok.c
// verify-options: --strict-init
// Pins: 0042 item 8 (question 83): a pointer local holding &x is not accepted as an argument of a marked parameter (static rule, over-strict, listed)
#include <cerastes.h>
#include <stddef.h>
void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0);

int f(void)
{
  CERASTES_UNINIT int x;
  int *q = &x;
  init(q);  // <- here
  return x;
}
