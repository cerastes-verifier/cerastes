// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/loop_atom_automatic.c
// Pins: 0040 item 5 (audit N5): owns(*p) written on an automatic place the function holds only as reads(*p): fails at entry
#include <cerastes.h>

void fill(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_reads(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
}
