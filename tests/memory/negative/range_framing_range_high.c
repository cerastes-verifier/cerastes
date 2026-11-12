// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/range_framing_range.c
// Pins: 0060 item 5: p[n] under owns_range(p, n) is one past the range: the contract reads it without owning it (permission at the clause)
// Pre-registered from record 0060 item 5, before the code
#include <cerastes.h>

int first(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[0] > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[0])
{
  (void)n;
  return p[0];
}

int last(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[n] > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[n - 1])
{
  return p[n - 1];
}
