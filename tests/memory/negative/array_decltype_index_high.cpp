// expect: error out-of-bounds @ L13
// mode: c++20
// twin: tests/memory/array_decltype.cpp
// Pins: 0063 item 14: decltype(a) b is an array of a's bound 3: i == 3 reaches the read of b[i]
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

int f(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
{
  int a[3] = {1, 2, 3};
  decltype(a) b = {1, 2, 3};
  return a[0] + b[i];  // <- here
}
