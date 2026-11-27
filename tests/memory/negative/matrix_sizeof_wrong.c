// expect: error check @ L11
// mode: c23, c++20
// twin: tests/memory/matrix_sizeof.c
// Pins: 0064 item 13: sizeof(m) of a two-row matrix of three ints is 24, so a check that it is 12 fails at the check (the value is the written bounds' product, not a fact the check assumes)
// Pre-registered from record 0064 item 13 (Part E), before any run: the one error
#include <cerastes.h>

int sizes(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  CERASTES_CHECK(sizeof(m) == 12);  // <- here
  return m[0][0];
}
