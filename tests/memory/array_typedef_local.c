// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 14 (flip A of M4.5e1e1 (the plan's section 10), was array_reject_typedef_local.c, rejected until now): a typedef of an array type at file scope, its bound checked as written at the typedef, and a local declared through it with a braced initializer; the program is unchanged, line for line
// Pre-registered from record 0063 item 14 (Part D), before any run: verifies
// The error twin is negative/array_typedef_local_index_high.c
#include <cerastes.h>

typedef int T3[3];

int f(void)
{
  T3 b = {1, 2, 3};
  return b[0];
}
