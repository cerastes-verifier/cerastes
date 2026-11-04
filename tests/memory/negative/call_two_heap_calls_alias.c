// expect: error unsequenced @ L17
// mode: c23, c++20
// twin: tests/memory/call_two_heap_calls.c
// Pins: 0045 item 6 (question 111): both calls may change x: unsequenced at the later call
#include <cerastes.h>
#include <stddef.h>

int bump(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && **pp < 100)
    CERASTES_ENSURES(r, cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == cerastes_old(**pp) + 1 && r == **pp);

int use(void)
{
  int x = 1;
  int *p = &x;
  int *q = &x;
  int y = bump(&p) + bump(&q);  // <- here
  return y;
}
