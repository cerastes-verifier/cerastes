// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/forall_must_init_ensures.c
// Pins: 0061 item 4 (twin): the second element is written 1, not 0: the quantified ensures fails at the closing brace (the flags of the must_init promise hold)
// Pre-registered from record 0061 item 4 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int i, const int *p, int n)
{
  return !(0 <= i && i < n) || p[i] == 0;
}

void zero2(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2) && cerastes_forall(zero_at, p, 2))
{
  p[0] = 0;
  p[1] = 1;
}
