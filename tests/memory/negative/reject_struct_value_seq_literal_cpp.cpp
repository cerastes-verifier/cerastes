// expect: rejected cerastes-unsequenced_modification @ L14
// twin: tests/memory/struct_value_cpp.cpp
// Pins: 0044 item 5 (P68 L5: no compiler warns): s = S{s.x++, 0} in C++ is over-strict, not undefined (C++17 sequences the right operand of = first): rejected by item 5's rule for the aggregate store of an element that writes the destination (M8, listed)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f()
{
  S s = {0, 0};
  s = S{s.x++, 0};  // <- here
  return s.x;
}
