// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/global_pointer_to_global.c
// Pins: 0041 item 6: the write through p = &g is the write of g (g == 5), so g == 6 fails
#include <cerastes.h>

static int g = 3;

void set_via_pointer(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g) && g == 6)
{
  int *p = &g;
  *p = 5;
}
