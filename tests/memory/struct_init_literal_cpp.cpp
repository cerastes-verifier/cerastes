// expect: verified
// expect: native-ok
// Pins: 0044 item 7 (flip of reject_struct_init_compound_literal.cpp): a compound literal as an initializer is a struct value in C++ (P67 H2: a plain aggregate)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int compound_init(int n)
    CERASTES_REQUIRES(n > 0 && n < 10)
    CERASTES_ENSURES(r, r == n)
{
  S s = (S){n, 2};
  return s.x;
}
