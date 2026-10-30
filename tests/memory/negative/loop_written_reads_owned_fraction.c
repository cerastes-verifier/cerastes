// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/loop_written_reads_owned.c
// Pins: 0040 item 5 (audit S3): the function holds only reads(*p), so the write under the atom has no write permission
#include <cerastes.h>
#include <stddef.h>

void write_owned(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads(*p) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_reads(*p))
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
