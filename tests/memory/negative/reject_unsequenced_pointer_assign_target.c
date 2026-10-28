// expect: rejected cerastes-unsequenced_modification @ L12
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0020 and 0032 item 9: *(p = &y) = *p reads p and modifies it with no sequence point between (undefined behaviour)
#include <cerastes.h>

int unsequenced_target(void)
{
  int x = 0;
  int y = 0;
  int *p = &x;
  *(p = &y) = *p;
  return x + y;
}
