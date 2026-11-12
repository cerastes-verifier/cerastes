// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/range_cell_contract_element_index.c
// Pins: 0060 item 5 (audit round 2, N1, g4b): the result equals the cell's value, so a check that they differ fails
// Pre-registered from record 0060 item 5 (audit round 2), before the code
#include <cerastes.h>
#include <stddef.h>

int g(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && n == 0 && p[n] > 0 && p[n] < 1000)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p[n]);

int use(void)
{
  int x = 5;
  int y = g(&x, 0);
  CERASTES_CHECK(y != x);
  return y;
}
