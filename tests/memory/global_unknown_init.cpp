// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0041 item 3 and 0017 item 4: an external-linkage global in C++ has an unknown value, and its initializer (here in range) is checked in every prologue
#include <cerastes.h>

signed char g = (signed char)100;

int main()
{
  g = 7;
  CERASTES_CHECK(g == 7);
  return 0;
}
