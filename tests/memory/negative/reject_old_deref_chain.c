// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/ptrptr_owns_inner.c
// Pins: 0045 item 1: *old(*pp), the object *pp designated on entry read on return: M8 (listed)
#include <cerastes.h>
#include <stddef.h>

void f(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp) && *cerastes_old(*pp) == 1)  // <- here
{
  (void)pp;
}
