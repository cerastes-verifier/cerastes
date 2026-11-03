// expect: error uninitialized @ L14
// mode: c++20
// twin: tests/memory/struct_init_self.cpp
// Pins: 0043 item 6, audit S7 (C++): the first element reads s.y, which the in-order initialization has not reached yet
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int self_member()
{
  S s = {s.y, 1};  // <- here
  return s.x;
}
