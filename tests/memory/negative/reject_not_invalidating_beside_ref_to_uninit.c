// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2: not_invalidating beside ref_to_uninit on one parameter: a pointee without a value has no pointers to keep, and the implicit ensures would read old(*p) (never lifted)
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter
#include <cerastes.h>

void f(int *p CERASTES_REF_TO_UNINIT CERASTES_NOT_INVALIDATING)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 0;
}
