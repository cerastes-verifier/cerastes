// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0063 item 14 (flip A of M4.5e1e1 (the plan's section 10), cut 3 not taken, was array_reject_const_bound.cpp): in C++ a bound that names a local constant (const int N = 4; int a[N];) is N's own checked constant, read through the bound's referrer (P82 (f)); unchanged, line for line (twin: negative/array_const_bound_index_high.cpp)
// Pre-registered from record 0063 item 14 (Part D), before any run: verifies
#include <cerastes.h>

int f(void)
{
  const int N = 4;
  [[maybe_unused]] int a[N];
  return N;
}
