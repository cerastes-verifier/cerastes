// expect: error lifetime @ L26
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 2 (audit round 2 S1): a pointer the loop assigns (q = id(s)) is any pointer after the loop; storing it through *pp is a use that needs its liveness, so it fails lifetime
#include <cerastes.h>
#include <stddef.h>

int *id(int *a)
    CERASTES_ENSURES(r, r == a);

int f(int n, int *s)
    CERASTES_REQUIRES(0 <= n && n <= 100 && cerastes_live(s))
    CERASTES_ENSURES(r, cerastes_live(s))
{
  int x = 0;
  int *q = NULL;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = id(s);
    i += 1;
  }
  *pp = q;
  return 0;
}
