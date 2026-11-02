// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 4: ref_to_uninit on a [[noreturn]] function needs nothing (it has no ensures); must_init there is rejected
#include <cerastes.h>

[[noreturn]] void bail(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p));

int use_bail(int c)
    CERASTES_ENSURES(r, r == 1)
{
  int x;
  if (c != 0) {
    bail(&x);
  }
  return 1;
}
