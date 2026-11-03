// expect: rejected cerastes-unsequenced_modification @ L15
// mode: c23, c++20
// twin: tests/memory/struct_seq_distinct.c
// Pins: 0043 item 12 (P66 M3): s.x++ + s.x: GCC -Wsequence-point (-Wall, both modes)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int postfix_plus(void)
{
  struct S s = {0, 0};
  return s.x++ + s.x;  // <- here
}
