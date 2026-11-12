// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/range_callee_keeps.c
// Pins: 0060 item 9: keep(&p[1]) never gives the slot back: the first marked row of the return fails (permission at the closing brace; a failed marked row ends its path, so one error)
// Pre-registered from record 0060 item 9, before the code
#include <cerastes.h>

void keep(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q));

void f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  keep(&p[1]);
}
