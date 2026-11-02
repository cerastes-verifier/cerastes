// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 4: both markers on one parameter: must_init implies ref_to_uninit (never lifted)
#include <cerastes.h>

void f(int *p CERASTES_REF_TO_UNINIT CERASTES_MUST_INIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 0;
}
