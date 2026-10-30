// expect: verified
// expect: native-ok
// c-only: in C++ a global with external linkage has an unknown value in main (global_linkage_cpp.cpp)
// Pins: 0041 item 3 (question 84): in C a global with external linkage has its initializer's value in main (nothing runs before main; a second definition is undefined, C23 6.9p5)
#include <cerastes.h>

int g = 5;

int main(void)
{
  CERASTES_CHECK(g == 5);
  g = 6;
  CERASTES_CHECK(g == 6);
  return 0;
}
