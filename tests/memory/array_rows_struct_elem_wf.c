// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 14 (Part E; the control of negative/array_rows_struct_elem_wf_division.c): a reads_range of p ties the ensures' read of p[0].a[1] (an element's array member) to the requires' read, so p[0].a[1] != 0 makes the divisor of 10 / p[0].a[1] non-zero in the ensures
// Pre-registered from record 0064 item 7 (Part E), before any run: verifies
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

int f(struct W *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1 && p[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == 0 || r == 10 / p[0].a[1]))
{
  (void)p;
  (void)n;
  return 0;
}
