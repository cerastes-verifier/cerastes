// expect: rejected cerastes-unsupported_expression @ L13
// twin: tests/memory/struct_value_cpp.cpp
// Pins: 0044 item 6 (M7): S(1, 2) as a value is a constructor call (parenthesized aggregate initialization)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f()
{
  S s = S(1, 2);  // <- here
  return s.x;
}
