// expect: verified
// expect: native-ok
// mode: c23
// c-only: typeof is a C23 keyword (C++20 has decltype: array_decltype.cpp)
// Pins: 0063 item 14 (flip A of M4.5e1e1 (the plan's section 10), was array_reject_typeof.c): an array local whose type is written typeof(a) is the array a's type, its bound checked where a was written; unchanged, line for line (twin: negative/array_typeof_index_high.c)
// Pre-registered from record 0063 item 14 (Part D), before any run: verifies
#include <cerastes.h>

int f(void)
{
  int a[3] = {1, 2, 3};
  typeof(a) b = {1, 2, 3};
  return a[0] + b[0];
}
