// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/matrix_row_pass.c
// Pins: 0064 items 5 and 8: the callee owns four cells and the row m[i] has three: the shape row of the call fails at the call (permission), as a scalar range past its array does (range_call_too_many.c), and the path ends
// Pre-registered from record 0064 items 5 and 8 (Part E), before any run: permission at the call
#include <cerastes.h>

void setrow(int *q)
    CERASTES_REQUIRES(cerastes_owns_range(q, 4))
    CERASTES_ENSURES(cerastes_owns_range(q, 4))
{
  q[0] = 1;
}

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  setrow(m[i]);  // <- here
  return m[i][2];
}
