// expect: error check @ L15
// mode: c23, c++20
// twin: tests/memory/ptrptr_alias_excluded.c
// Pins: 0040 item 2: the aliasing trap: *pp may be p, so *p = 2 may overwrite **pp
#include <cerastes.h>
#include <stddef.h>

void trap(int **pp, int *p, int *s)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*p) && cerastes_owns(*s) && (*pp == p || *pp == s))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(*p) && cerastes_owns(*s))
{
  (void)s;
  **pp = 1;
  *p = 2;
  CERASTES_CHECK(**pp == 1);
}
