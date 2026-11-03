// expect: rejected cerastes-unsupported_expression @ L14
// twin: tests/memory/struct_value_cpp.cpp
// Pins: 0044 item 6 (M7): t = {1, 2}; is a call of operator= with a braced list
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f()
{
  S t = {0, 0};
  t = {1, 2};  // <- here
  return t.x;
}
