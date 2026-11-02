// expect: rejected cerastes-unsupported_declaration @ L16
// mode: c++20
// twin: tests/memory/struct_init_cpp.cpp
// Pins: 0043 item 15, audit round 2: a compound literal as an initializer is a struct value (M4.5c2): rejected at the declaration
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int compound_init(int n)
    CERASTES_REQUIRES(n > 0 && n < 10)
    CERASTES_ENSURES(r, r == n)
{
  S s = (S){n, 2};  // <- here
  return s.x;
}
