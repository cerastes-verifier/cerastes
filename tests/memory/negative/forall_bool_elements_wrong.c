// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/forall_bool_elements.c
// Pins: 0061 item 7 (twin): the second flag is cleared: the quantified bool ensures fails at the closing brace
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool all_true(int i, const bool *a, int n)
{
  return !(0 <= i && i < n) || a[i];
}

void set_true2(bool *p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2) && cerastes_forall(all_true, p, 2))
{
  p[0] = true;
  p[1] = false;
}
