// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/and_assign.c
// Pins: 0012 item 12 and 0020 item 5 (M4.3a audit note 5): EDG folds `1 || x++`, so its increment is never evaluated and cannot be encoded; rejected
#include <cerastes.h>

int f(int a)
    CERASTES_REQUIRES(a < 2147483647)
{
  int x = a;
  int y = 1 || x++;
  return x + y;
}
