// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 5: s.y is read before it is written: uninitialized, naming the member
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int second(void)
{
  struct S s;
  s.x = 3;
  return s.y;  // <- here
}
