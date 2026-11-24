// expect: rejected cerastes-unsupported_annotation @ L15
// expect: rejected cerastes-unsupported_annotation @ L16
// mode: c23, c++20
// twin: tests/memory/pure_keep_fact_cell.c
// Pins: 0062 item 1 (audit round 3 R3-S4; M4.5e2): the application under ! on the left of ||: a guard reads no memory and an application counts as a read, so an application in the guard of an atom is unsupported_annotation, in the requires and in the ensures
// Pre-registered from record 0062 (Part C), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int pos(int x)
{
  return x > 0;
}

int f(int *p, int n)
    CERASTES_REQUIRES(!pos(n) || cerastes_owns(*p))
    CERASTES_ENSURES(!pos(n) || cerastes_owns(*p))
{
  return 0;
}
