// expect: rejected cerastes-unsupported_expression @ L9
// mode: c23, c++20
// twin: tests/memory/ptrptr_const_middle.c
// Pins: 0040 item 2 (re-audit of audit round 2's B1): a cast of the int ** result to int *const * in an ensures: rejected
#include <cerastes.h>
#include <stddef.h>

int **unconst(int *const *cpp)
    CERASTES_ENSURES(r, (int *const *)r == cpp);

int f(void)
{
  int x = 1;
  int y = 2;
  int *const p = &x;
  int **pp = unconst(&p);
  *pp = &y;
  CERASTES_CHECK(p == &y);
  return *p;
}
