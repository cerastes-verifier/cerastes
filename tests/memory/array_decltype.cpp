// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0063 item 14 (flip A of M4.5e1e1 (the plan's section 10), was array_reject_decltype.cpp): decltype(a) of an array local is the array a's type; unchanged, line for line (twin: negative/array_decltype_index_high.cpp)
// Pre-registered from record 0063 item 14 (Part D), before any run: verifies
#include <cerastes.h>

int f(void)
{
  int a[3] = {1, 2, 3};
  decltype(a) b = {1, 2, 3};
  return a[0] + b[0];
}
