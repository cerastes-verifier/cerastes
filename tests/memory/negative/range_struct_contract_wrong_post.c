// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/range_struct_contract.c
// Pins: 0064 item 7: first_x's postcondition says r == 0 where p[0].x is only known to be non-negative: the postcondition fails at the return (once)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: postcondition at the return
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  long y;
};

int first_x(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[0].x >= 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == 0)
{
  (void)n;
  return p[0].x;  // <- here
}

void bump_y(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0].x == cerastes_old(p[0].x) && p[0].y == 7)
{
  (void)n;
  p[0].y = 7;
}

int dec(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[0].x >= 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
    CERASTES_DECREASES(p[0].x)
{
  (void)p;
  return n;
}
