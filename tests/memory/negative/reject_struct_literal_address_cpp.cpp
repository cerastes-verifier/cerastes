// expect: rejected cerastes-unsupported_expression @ L13
// twin: tests/memory/struct_literal_value_cpp.cpp
// Pins: 0044 item 7 (P68 K3, P67 H3): &(S){1, 2} in C++ is a temporary: clang and GCC refuse it, EDG accepts it silently, so the rejection is cerastes's
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f()
{
  S *p = &(S){1, 2};  // <- here
  return p->x;
}
