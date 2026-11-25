// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 1 and 9 (flip A of M4.5e1e1, the staging pin static_array_reject_use.c retired): a static array local that the function reads, its implicit atom holding the element and its flag (the initializer {5} writes element 0); unchanged, line for line (twin negative/static_array_use_check_wrong.c)
// Pre-registered from record 0063 item 9 (Part B), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

int f(void)
{
  static int t[2] = {5};
  return t[0];
}
