// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/range_reads.c
// Pins: 0060 item 4 and 0028 item 11: a function that only reads p[0..n) writes p[i]: the write's permission check fails (the amount is below write); the borrowed range is still handed back
// Pre-registered from record 0060 items 4 and 9, before the code
#include <cerastes.h>

void put(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_reads_range(p, n))
{
  p[i] = 1;
}
