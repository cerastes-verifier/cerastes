// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/range_loop_search.c
// Pins: 0060 item 11: the ensures claims p[r] != x where the loop returned the index of x: the postcondition fails at that return only (the -1 return is fine)
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

int search(const int *p, int n, int x)
    CERASTES_REQUIRES(cerastes_reads_range(p, n))
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == -1 || (0 <= r && r < n && p[r] != x)))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    if (p[i] == x) {
      return i;
    }
    i++;
  }
  return -1;
}
