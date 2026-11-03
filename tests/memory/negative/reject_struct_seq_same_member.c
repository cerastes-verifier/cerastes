// expect: rejected cerastes-unsequenced_modification @ L15
// mode: c23, c++20
// twin: tests/memory/struct_seq_distinct.c
// Pins: 0043 item 12 (P66 M1): s.x read and written with no sequence point between: GCC's -Wsequence-point warns at -Wall in both modes, so the shape is rejected like the compilers
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int same_member(void)
{
  struct S s = {0, 0};
  int r = s.x + (s.x = 1);  // <- here
  return r;
}
