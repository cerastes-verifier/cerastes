// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/mixed_atoms.c
// Pins: 0037 item 10: the value ensures is off by one
#include <cerastes.h>
#include <stddef.h>

int sum3(int *a, const int *b, int n)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_reads(*b) && n > 0 && n < 100 && *a >= 0 && *a < 100 && *b >= 0 && *b < 100)
    CERASTES_ENSURES(r, cerastes_owns(*a) && cerastes_reads(*b) && *a == cerastes_old(*a) && *b == cerastes_old(*b) && r == *a + *b + n)
{
  return *a + *b + n + 1;
}

int use_sum3(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 1;
  int y = 2;
  return sum3(&x, &y, 3);
}
