// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/array_rows_param.c
// Pins: 0064 items 7 and 8: the call says three rows and the caller holds two: the shape row of the call fails at the call (permission), as for a range over structures (range_struct_owns_call_past.c)
// Pre-registered from record 0064 items 7 and 8 (Part E), before any run: permission at the call
#include <cerastes.h>

void put(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  (void)n;
  a[0][0] = 1;
}

int use(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  put(m, 3);  // <- here
  return m[0][0];
}
