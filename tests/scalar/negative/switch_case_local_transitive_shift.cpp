// expect: error shift @ L10
// twin: tests/scalar/switch_case_named.cpp
// Pins: 0017 item 6: a local const reached through another local const used as a case value is checked at the entry, in order
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(x <= 10)
{
  if (x > 100) {
    const int a = 1 << 40;
    const int k = a + 1;
    switch (x) {
    case k:
      return 1;
    default:
      return 2;
    }
  }
  return 0;
}
