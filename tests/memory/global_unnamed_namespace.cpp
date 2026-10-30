// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0041 item 3: a variable of the unnamed namespace has internal linkage: its initializer's value is known in main in C++
#include <cerastes.h>

namespace {
int g = 5;
}

int main()
{
  CERASTES_CHECK(g == 5);
  g = 6;
  CERASTES_CHECK(g == 6);
  return 0;
}
