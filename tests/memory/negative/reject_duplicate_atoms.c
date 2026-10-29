// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 items 5 and 15: two atoms of one kind on one place in one clause list are rejected (the requires would be infeasible; never lifted)
#include <cerastes.h>
#include <stddef.h>

void f(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p))
{
  (void)p;
}
