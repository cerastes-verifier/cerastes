// expect: error invariant-preserved @ L19
// mode: c23, c++20
// twin: tests/memory/forall_copy_loop.c
// Pins: 0061 item 9 (S45 (b) copy_loop_fail): the loop stores 0 instead of src[i]: the two-class clause is not preserved (one error)
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool same_at(int k, const int *a, const int *b, int n)
{
  return !(0 <= k && k < n) || a[k] == b[k];
}

void copy(int *dst, const int *src, int n)
    CERASTES_REQUIRES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n))
    CERASTES_ENSURES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n) && cerastes_forall(same_at, dst, src, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(same_at, dst, src, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dst[i] = 0;
    i++;
  }
}
