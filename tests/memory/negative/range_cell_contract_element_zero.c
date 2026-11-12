// expect: error precondition @ L25
// mode: c23, c++20
// twin: tests/memory/range_cell_contract_element.c
// Pins: 0060 item 5 (audit round 2, N1, g2): the element read p[0] > 0 of the requires fails for the cell holding 0: precondition at that call
// Pre-registered from record 0060 item 5 (audit round 2), before the code
#include <cerastes.h>
#include <stddef.h>

int g(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p[0] > 0 && p[0] < 1000)
    CERASTES_ENSURES(r, cerastes_owns(*p) && p[0] == cerastes_old(p[0]) + 1 && r == p[0]);

int use(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 5;
  int y = g(&x);
  CERASTES_CHECK(x == 6);
  return y;
}

void bad(void)
{
  int z = 0;
  g(&z);
}
