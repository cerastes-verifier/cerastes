// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11 (S44 (g) copy): an owned range and a read range in one loop (dst[i] = src[i]): two range places, the liveness case split of item 7 at entry, both given back
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void copy(int *dst, const int *src, int n)
    CERASTES_REQUIRES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n))
    CERASTES_ENSURES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    dst[i] = src[i];
    i++;
  }
}
