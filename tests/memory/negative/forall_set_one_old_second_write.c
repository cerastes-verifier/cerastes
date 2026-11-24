// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/forall_set_one_old.c
// Pins: 0061 item 8 (S45 (e) twin): the body also writes a[0], which is not j for every j: the quantified keep-its-value clause fails at the closing brace
// Pre-registered from record 0061 item 8 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool kept(int i, const int *a, int n, int j)
{
  return !(0 <= i && i < n && i != j) || a[i] == cerastes_old(a[i]);
}

void set_one(int *a, int n, int j, int v)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 0 <= j && j < n)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[j] == v && cerastes_forall(kept, a, n, j))
{
  (void)n;
  a[j] = v;
  a[0] = 7;
}
