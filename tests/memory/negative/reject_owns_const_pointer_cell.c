// expect: rejected cerastes-unsupported_annotation @ L10
// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 2 (re-audit of audit round 2's B1): owns of a const pointer cell (*pp of an int *const * parameter): rejected
#include <cerastes.h>
#include <stddef.h>

void repoint(int *const *pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == q);

int f(void)
{
  int x = 1;
  int y = 2;
  int *const p = &x;
  repoint(&p, &y);
  CERASTES_CHECK(p == &y);
  return *p;
}
