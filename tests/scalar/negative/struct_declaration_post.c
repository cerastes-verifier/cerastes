// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/scalar/struct_declaration.c
// Pins: flip (a) of question 93, record 0043: a struct declaration beside a scalar function whose ensures is wrong
#include <cerastes.h>

struct point {
  int x;
  int y;
};

int ident(int a)
    CERASTES_REQUIRES(-1000 < a && a < 1000)
    CERASTES_ENSURES(r, r == a + 1)
{
  return a;  // <- here
}
