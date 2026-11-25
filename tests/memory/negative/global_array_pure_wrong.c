// expect: error check @ L21
// mode: c23, c++20
// twin: tests/memory/global_array_forall_pure.c
// Pins: 0063 item 1: the pure function's contract bounds the count by n = 3 only: count_to(b, 3) <= 2 is not a fact (a pure application over a named array global in a check)
// Pre-registered from record 0063 item 1 (Part B), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE int count_to(const bool *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, 0 <= r && r <= n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : count_to(a, n - 1) + (a[n - 1] ? 1 : 0);
}

static bool b[3] = {true, false, true};

int main(void)
{
  CERASTES_CHECK(count_to(b, 3) <= 2);  // <- here
  return b[0] ? 0 : 1;
}
