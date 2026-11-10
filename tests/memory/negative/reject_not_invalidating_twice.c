// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2: the marker twice on one parameter (never lifted)
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter
#include <cerastes.h>

void f(int *p CERASTES_NOT_INVALIDATING CERASTES_NOT_INVALIDATING)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 0;
}
