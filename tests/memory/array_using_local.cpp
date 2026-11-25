// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0063 item 14 (flip A of M4.5e1e1 (the plan's section 10), was array_reject_using_local.cpp): a using-alias of an array type at file scope and a local declared through it; the program is unchanged, line for line
// Pre-registered from record 0063 item 14 (Part D), before any run: verifies
// The error twin is negative/array_using_local_index_high.cpp
#include <cerastes.h>

using T3 = int[3];

int f(void)
{
  T3 b = {1, 2, 3};
  return b[0];
}
