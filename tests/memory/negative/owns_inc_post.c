// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 item 10: the callee promises +1, the body adds 2: the value ensures fails at the return
#include <cerastes.h>
#include <stddef.h>

int inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 2;
  return *p;
}

int use_inc(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 5;
  int v = inc(&x);
  CERASTES_CHECK(x == 6);
  return v;
}
