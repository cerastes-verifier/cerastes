// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/range_loop_copy.c
// Pins: 0060 item 11: the loop writes the read-only range (src[i] = 0): the write's permission check fails at the store, one error
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

void copy(int *dst, int *src, int n)
    CERASTES_REQUIRES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n))
    CERASTES_ENSURES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    src[i] = 0;
    dst[i] = 0;
    i++;
  }
}
