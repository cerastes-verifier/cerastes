// expect: error unsequenced @ L19
// mode: c23, c++20
// twin: tests/memory/call_beside_other_cell.c
// Pins: 0039 item 4: x += bump(&x): the read of the target is an access C does not order with the call (re-expects the retired pin of 0037 items 8 and 15)
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int compound(void)
{
  int x = 1;
  x += bump(&x);
  return x;
}
