// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 9 and 11 (Part C2, audit round C1's finding 5 for a global array of structures; f__wf's ghosts, one per leaf path and slot): the control of negative/global_array_struct_wf_division.c. A reads(g) ties the ensures' read of g[0].x to the requires' read (the array is unchanged), so g[0].x != 0 in the requires makes the divisor of 10 / g[0].x non-zero in the ensures
// Pre-registered from record 0064 items 9 and 11 and the audit's finding 5 (w04's shape), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {3, 4}};

int f(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[0].x != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[0].x))
{
  return g[0].x * 0;
}
