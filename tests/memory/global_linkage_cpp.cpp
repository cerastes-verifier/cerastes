// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0041 item 3 (question 84): in C++ a global with external linkage has an unknown, initialized value in main (another unit's dynamic initializer may write it): main may write it and then know it
#include <cerastes.h>

int g = 5;

int main()
{
  g = 6;
  CERASTES_CHECK(g == 6);
  return 0;
}
