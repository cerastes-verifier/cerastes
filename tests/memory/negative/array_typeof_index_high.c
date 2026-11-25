// expect: error out-of-bounds @ L14
// mode: c23
// c-only: typeof is a C23 keyword
// twin: tests/memory/array_typeof.c
// Pins: 0063 item 14: typeof(a) b is an array of a's bound 3: i == 3 reaches the read of b[i]
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

int f(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
{
  int a[3] = {1, 2, 3};
  typeof(a) b = {1, 2, 3};
  return a[0] + b[i];  // <- here
}
