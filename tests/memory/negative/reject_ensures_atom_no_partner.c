// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 items 6 and 15: an ensures atom with no requires atom of the same kind on the same place is rejected (re-expected by b1c's owning results)
#include <cerastes.h>
#include <stddef.h>

void f(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(cerastes_live(p) && cerastes_owns(*p))
{
  (void)p;
}
