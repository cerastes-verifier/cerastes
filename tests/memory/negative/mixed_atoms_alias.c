// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/mixed_atoms.c
// Pins: 0037 item 8: the owns and the reads atom may not name one cell: the pairwise separation fails at the call
#include <cerastes.h>
#include <stddef.h>

int sum3(int *a, const int *b, int n)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_reads(*b) && n > 0 && n < 100 && *a >= 0 && *a < 100 && *b >= 0 && *b < 100)
    CERASTES_ENSURES(r, cerastes_owns(*a) && cerastes_reads(*b) && *a == cerastes_old(*a) && *b == cerastes_old(*b) && r == *a + *b + n)
{
  return *a + *b + n;
}

int use_sum3(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 1;
  return sum3(&x, &x, 3);
}
