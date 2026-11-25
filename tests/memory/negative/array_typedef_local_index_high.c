// expect: error out-of-bounds @ L14
// mode: c23, c++20
// twin: tests/memory/array_typedef_local.c
// Pins: 0063 item 14: a local declared through the typedef T3 = int[3] has the array's bound 3: i == 3 reaches the read (out-of-bounds at the access)
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

typedef int T3[3];

int f(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
{
  T3 b = {1, 2, 3};
  return b[i];  // <- here
}
