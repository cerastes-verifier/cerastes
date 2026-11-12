// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/range_partners.c
// Pins: 0060 item 3: same_n claims r == 0 where it returns n: the postcondition fails at the return (the partners themselves are fine)
// Pre-registered from record 0060 item 3, before the code
#include <cerastes.h>

int same_n(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == 0)
{
  (void)p;
  return n;
}

int plus_one(int *p, int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_REQUIRES(cerastes_owns_range(p, n + 1))
    CERASTES_ENSURES(r, cerastes_owns_range(p, n + 1) && r == n)
{
  (void)p;
  return n;
}

int cast_n(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, (long)n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, (long)n) && r == n)
{
  (void)p;
  return n;
}
