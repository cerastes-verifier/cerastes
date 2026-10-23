// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/assign_condition_shapes.c
// Pins: 0020 item 5: the condition's assignment stores a
#include <cerastes.h>

int negated(int a)
    CERASTES_ENSURES(r, r == (a == 0 ? 1 : a))
{
  int x;
  if (!(x = a)) {
    return x + 2;
  }
  return x;
}

int and_cond(int a, int b)
    CERASTES_REQUIRES(-2147483647 <= a)
    CERASTES_ENSURES(r, r == (b != 0 ? a : -a))
{
  int x;
  if ((x = a) && b) {
    return x;
  }
  return -x;
}

int compound_cond(int a, int b)
    CERASTES_REQUIRES(0 <= b && a <= 2147483647 - b)
    CERASTES_ENSURES(r, r == (a + b != 0 ? a + b : 100))
{
  int x = b;
  if (x += a) {
    return x;
  }
  return 100;
}

int comma_cond(int a)
    CERASTES_ENSURES(r, r == (a != 0 ? a : 100))
{
  int x;
  if (x = a, x) {
    return x;
  }
  return 100;
}
