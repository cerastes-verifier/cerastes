// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 2 (re-audit of audit round 2's B1): a cast of *pp to const int * in an ensures: rejected
#include <cerastes.h>
#include <stddef.h>

void set(int **pp, const int *v)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && (const int *)*pp == v);

int f(void)
{
  const int c = 1;
  int x = 0;
  int *p = &x;
  set(&p, &c);
  *p = 2;
  CERASTES_CHECK(c == 2);
  return 0;
}
