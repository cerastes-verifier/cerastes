// expect: rejected cerastes-unsupported_expression @ L9
// mode: c23, c++20
// twin: tests/memory/ptrptr_const_middle.c
// Pins: 0040 item 2 (audit round 2 B1): a contract equates an int ** result with an int *const * parameter (r == cpp): the caller could write the const pointer; rejected at the contract
#include <cerastes.h>
#include <stddef.h>

int **unconst(int *const *cpp)
    CERASTES_ENSURES(r, r == cpp);

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
