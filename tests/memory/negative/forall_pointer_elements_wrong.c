// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/forall_pointer_elements.c
// Pins: 0061 item 7 (twin): the second pointer is never cleared: the quantified ensures fails at the closing brace
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool all_null(int i, int **a, int n)
{
  return !(0 <= i && i < n) || a[i] == NULL;
}

void clear2(int **p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2) && cerastes_forall(all_null, p, 2))
{
  p[0] = NULL;
}
