// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 3: partners by structural equality of the count: n and n, n + 1 and n + 1, (long)n and (long)n are the same count (a written cast keeps its meaning and is compared as written)
// Pre-registered from record 0060 item 3, before the code: verifies
#include <cerastes.h>

int same_n(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
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
