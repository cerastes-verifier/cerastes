// expect: error unsequenced @ L30
// mode: c23, c++20
// twin: tests/memory/range_call_place.c
// Pins: 0060 item 10 (S44 (i)): two calls in one expression whose ranges (&a[0], 2) and (&a[1], 2) overlap: the pair row fails, unsequenced at the later call, one error
// Pre-registered from record 0060 item 10 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

int sumo(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n == 2)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == 7)
{
  (void)p;
  (void)n;
  return 7;
}

int read_beside(void)
    CERASTES_ENSURES(r, r == 8)
{
  int a[4] = {1, 2, 3, 4};
  int x = a[0] + sumo(&a[1], 2);
  return x;
}

int two_calls(void)
    CERASTES_ENSURES(r, r == 14)
{
  int a[4] = {1, 2, 3, 4};
  int x = sumo(&a[0], 2) + sumo(&a[1], 2);
  return x;
}
