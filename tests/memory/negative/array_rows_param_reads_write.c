// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/array_rows_param.c
// Pins: 0064 item 7 and 0028 item 11 (as range_struct_reads_write.c): a function that only reads the rows of a[0..n) writes a[0][1]: the write's permission check fails (the amount is below write)
// Pre-registered from record 0064 item 7 (Part E), before any run: permission at the write
#include <cerastes.h>

void bad(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)n;
  a[0][1] = 1;  // <- here
}
