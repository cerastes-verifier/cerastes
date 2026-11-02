// expect: rejected cerastes-unsequenced_modification @ L19
// mode: c23, c++20
// twin: tests/memory/struct_seq_distinct.c
// Pins: 0043 item 12 (P66 M9): u.t.x read and written: GCC -Wsequence-point (-Wall, both modes)
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

int nested_same(void)
{
  struct T u = {{1, 2}, 3};
  return u.t.x + (u.t.x = 1);  // <- here
}
