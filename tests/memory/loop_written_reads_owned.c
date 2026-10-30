// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5 (audit S3): a reads(*p) atom takes everything the function holds of its place: the function owns it whole, so the write in the loop verifies
#include <cerastes.h>
#include <stddef.h>

void write_owned(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_reads(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    i += 1;
  }
}
