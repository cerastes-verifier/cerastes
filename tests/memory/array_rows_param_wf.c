// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 13 (Part E; f__wf's ghosts, one per leaf path and slot: the control of negative/array_rows_param_wf_division.c): a reads_range of a ties the ensures' read of a[0][0] to the requires' read (the range is unchanged), so a[0][0] != 0 in the requires makes the divisor of 10 / a[0][0] non-zero in the ensures
// Pre-registered from record 0064 items 7 and 13 (Part E), before any run: verifies
#include <cerastes.h>

int f(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 1 && a[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && (r == 0 || r == 10 / a[0][0]))
{
  (void)n;
  return a[0][0] * 0;
}
