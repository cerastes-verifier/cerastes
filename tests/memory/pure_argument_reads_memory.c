// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 4 (cut 3): an argument that reads memory (a[n - 1]) is an expression of the site, evaluated once in the contract's state with its own framing row: the ensures applies small to the last element
// Pre-registered from record 0062 (Part C), before the code: verifies
#include <cerastes.h>

CERASTES_PURE bool small(int x)
{
  return -100 < x && x < 100;
}

bool last_small(const int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == small(a[n - 1]))
{
  return -100 < a[n - 1] && a[n - 1] < 100;
}
