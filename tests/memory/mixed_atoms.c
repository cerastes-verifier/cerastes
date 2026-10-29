// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 items 6, 8: an owns atom and a reads atom in one contract, each with its own amount; separation of the two on one field
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
  int y = 2;
  return sum3(&x, &y, 3);
}
