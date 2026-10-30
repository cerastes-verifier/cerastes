// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 12 (replaces tests/scalar/negative/reject_static_local.c, retired by question 81): a static local that the function never uses is accepted
#include <cerastes.h>

int counter(int a)
{
  [[maybe_unused]] static int count = 0;
  return a;
}
