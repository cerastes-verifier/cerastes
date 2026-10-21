// expect: verified
// expect: native-ok
// mode: c23, c++20
// break leaves a loop by another exit; a switch with grouped labels and a default.
// tag::source[]
#include <cerastes.h>

// The smallest divisor of n above 1 (n itself when n is prime).
int smallest_divisor(int n)
    CERASTES_REQUIRES(2 <= n && n <= 10000)
    CERASTES_ENSURES(r, 2 <= r && r <= n && n % r == 0)
{
  int d = 2;
  CERASTES_INVARIANT(2 <= d && d <= n)
  CERASTES_DECREASES(n - d)
  while (d < n) {
    if (n % d == 0) {
      break;
    }
    d += 1;
  }
  return d;
}

// The days in a month of a year that is not a leap year.
int days_in_month(int month)
    CERASTES_REQUIRES(1 <= month && month <= 12)
    CERASTES_ENSURES(r, 28 <= r && r <= 31)
{
  int days = 31;
  switch (month) {
  case 2:
    days = 28;
    break;
  case 4:
  case 6:
  case 9:
  case 11:
    days = 30;
    break;
  default:
    break;
  }
  return days;
}
// end::source[]
