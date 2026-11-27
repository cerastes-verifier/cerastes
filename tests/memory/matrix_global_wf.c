// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 9, 11 and 14 (Part E, question 362: contract reads g[e][e2] of a global matrix are built in slice 3; the control of negative/matrix_global_wf_division.c): a reads atom of g ties the ensures' read of g[0][0] to the requires' read, so g[0][0] != 0 makes the divisor non-zero in the ensures
// Pre-registered from record 0064 items 9 and 11 (Part E), before any run: verifies
#include <cerastes.h>

static int g[2][3] = {{1, 2, 3}, {4, 5, 6}};

int f(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[0][0]))
{
  return g[0][0] * 0;
}
