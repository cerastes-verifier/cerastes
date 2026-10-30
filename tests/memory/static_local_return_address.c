// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 12: return &s of a static local is accepted (static storage outlives the call: R14 does not fire); no caller uses the result (a use of an unframed pointer result is a lifetime false alarm until M4.5d2, static_local_return_address_use.c)
#include <cerastes.h>

int *where(void)
{
  static int s = 5;
  return &s;
}

int main(void)
{
  return 0;
}
