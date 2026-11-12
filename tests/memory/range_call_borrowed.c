// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 8: a range a function owns passed on whole, shorter by one, and as (&p[k], n - k) (k == n is the empty tail one past the end): the callee's rows read the caller's own range, whose pure line gives the shape row, and the range comes back
// Pre-registered from record 0060 item 8 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void touch(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
}

void whole(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n < 100)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  touch(p, n);
}

void shorter(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  touch(p, n - 1);
}

void tail(int *p, int n, int k)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= k && k <= n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  touch(&p[k], n - k);
}
