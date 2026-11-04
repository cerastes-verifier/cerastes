// expect: error unsequenced @ L16
// mode: c23, c++20
// twin: tests/memory/call_beside_access_heap.c
// Pins: 0045 item 6 (question 111): **pp may be x, which the expression also reads: unsequenced at the call
#include <cerastes.h>
#include <stddef.h>

int bump(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && **pp < 100)
    CERASTES_ENSURES(r, cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == cerastes_old(**pp) + 1 && r == **pp);

int use(void)
{
  int x = 1;
  int *p = &x;
  int y = x + bump(&p);  // <- here
  return y;
}
