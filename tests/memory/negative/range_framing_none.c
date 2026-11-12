// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/range_framing_range.c
// Pins: 0060 item 5: a contract read of p[0] with no atom for p: permission at the clause, in source terms
// Pre-registered from record 0060 item 5, before the code
#include <cerastes.h>

int f(int *p, int n)
    CERASTES_REQUIRES(n > 0 && p[0] > 0)
{
  (void)p;
  return n;
}
