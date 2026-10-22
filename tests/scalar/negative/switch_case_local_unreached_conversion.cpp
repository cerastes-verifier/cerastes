// expect: error conversion @ L10
// twin: tests/scalar/switch_case_named.cpp
// Pins: 0017 item 6: a local const used as a case value is checked at the entry of the function, although its branch is never taken
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(x <= 10)
{
  if (x > 100) {
    const int k = (signed char)300;
    switch (x) {
    case k:
      return 1;
    default:
      return 2;
    }
  }
  return 0;
}
