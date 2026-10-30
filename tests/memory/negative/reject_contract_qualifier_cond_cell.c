// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/memory/ptrptr_set.c
// Pins: 0040 item 2 (re-audit of audit round 2's B1): a ?: of *pp and a const int * parameter in an ensures: one arm is converted, rejected
#include <cerastes.h>
#include <stddef.h>

void set(int **pp, const int *v)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp != NULL ? *pp : v) == v && *pp != NULL);

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
