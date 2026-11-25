// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/global_array_forall_pure.c
// Pins: 0063 item 1: the same check over an array global holding a negative element: the quantified fact is false (the one failing check)
// Pre-registered from record 0063 item 1 (Part B), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

static int g[4] = {1, 2, -3, 4};

int main(void)
{
  CERASTES_CHECK(cerastes_forall(nonneg, g, 4));  // <- here
  return g[0] - 1;
}
