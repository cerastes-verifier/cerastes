// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 (audit round C1, finding 5; f__wf's ghosts, one per leaf path and slot): the control of negative/range_struct_wf_division.c. A reads_range of p ties the ensures' read of p[0].x to the requires' read (the range is unchanged), so p[0].x != 0 in the requires makes the divisor of 10 / p[0].x non-zero in the ensures
// Pre-registered from record 0064 item 7 and the audit's finding 5 (w04), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

int f(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1 && p[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == 0 || r == 10 / p[0].x))
{
  (void)p;
  (void)n;
  return 0;
}
