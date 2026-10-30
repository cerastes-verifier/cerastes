// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/memory/loop_pick_pointer.c
// Pins: 0038 item 10: &y in a CERASTES_CHECK of a variable the code never takes the address of is no cell; rejected, not read
#include <cerastes.h>

int never_taken(void)
{
  int x = 0;
  int y = 0;
  int *p = &x;
  CERASTES_CHECK(p != &y);
  return *p + y;
}
