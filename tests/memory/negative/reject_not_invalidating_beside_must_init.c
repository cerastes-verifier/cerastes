// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2: not_invalidating beside must_init on one parameter, in the leading position for one of them (never lifted)
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter
#include <cerastes.h>

void f(CERASTES_NOT_INVALIDATING int *p CERASTES_MUST_INIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}
