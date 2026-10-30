// expect: error division-by-zero @ L10
// mode: c23, c++20
// twin: tests/memory/result_value_reads.c
// Pins: 0040 item 4 (audit round 2 S4): with owns(*p) the post-state value of *r is not the entry value, so 10 / *r may divide by zero
#include <cerastes.h>
#include <stddef.h>

int *pick(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p > 0)
    CERASTES_ENSURES(r, r == p && cerastes_owns(*p) && 10 / *r >= 0)
{
  return p;
}

int use(void)
{
  int x = 3;
  return *pick(&x);
}
