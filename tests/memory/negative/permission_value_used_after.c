// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/pointer_scope_inner.c
// Pins: audit round 1 of M4.5b1a, 0032 item 6: the single-error claim: the value of the failed read is used after it (+ 1, returned) and still only the permission is reported
#include <cerastes.h>

int inner_scope(int a)
    CERASTES_REQUIRES(-1000 < a && a < 1000)
{
  int x = a;
  int *p = &x;
  {
    int y = 1;
    p = &y;
  }
  int z = *p + 1;
  return z;
}
