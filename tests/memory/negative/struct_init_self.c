// expect: error uninitialized @ L14
// c-only: the elements of an aggregate initializer are evaluated before any is stored in C and one by one in C++: the same shape has the other outcome there
// twin: tests/memory/struct_init_self.cpp
// Pins: 0043 item 6, audit S7 (C): C evaluates every element before it stores any, so the read of s.x sees the uninitialized leaf (the same shape is verified in C++, struct_init_self.cpp)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int self_member(void)
{
  struct S s = {1, s.x};  // <- here
  return s.y;
}
