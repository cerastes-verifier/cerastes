// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/global_pointer_to_global.c
// Pins: 0041 item 6: &g is a valid pointer anywhere, but writing through it needs the permission to g's cell, which this function does not hold
#include <cerastes.h>

static int g = 3;

void set_via_pointer(void)
{
  int *p = &g;
  *p = 5;
}
