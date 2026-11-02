// expect: rejected cerastes-uninit_ref_initialized @ L14
// mode: c23, c++20
// twin: tests/memory/init_strict_ok.c
// verify-options: --strict-init
// Pins: 0042 item 8 (question 83): a null constant is not &x of a marked local (over-strict, listed)
#include <cerastes.h>
#include <stddef.h>
void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0);

void f(void)
{
  init(NULL);  // <- here
}
