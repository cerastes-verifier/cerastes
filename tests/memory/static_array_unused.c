// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 1 and 9 (flip A of M4.5e1e1, was array_reject_static_local.c): a static array local with a braced initializer, the function not touching it; unchanged, line for line (twin negative/static_array_bound_wrap.c)
// Pre-registered from record 0063 item 9 (Part B), before any run: verifies
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] static int a[3] = {1, 2, 3};
  return 0;
}
