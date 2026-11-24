// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/pure_argument_reads_memory.c
// Pins: 0062 item 4: the body drops the lower bound of small: the postcondition error, at the return
// Pre-registered from record 0062 (Part C), before the code: postcondition
#include <cerastes.h>

CERASTES_PURE bool small(int x)
{
  return -100 < x && x < 100;
}

bool last_small(const int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == small(a[n - 1]))
{
  return a[n - 1] < 100;
}
