// expect: verified
// expect: native-ok
// mode: c23, c++20
// twin-exempt: the cross-check's exemption has no failing counterpart: the only error would be the exempt assertion itself, which nothing reports before M4.5f
// Pins: 0025 item 5 and 0024 note 6 (M4.4b2): the cross-check exempts the region of a pure (or predicate) routine, whose body no encoder reads before M4.5f
#include <cerastes.h>

// a pure function's body is not encoded before M4.5f, so a block assertion in it is exempt from the
// cross-check (it is not an assertion the encoder was supposed to see)
CERASTES_PURE int twice(int x)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  return x + x;
}

int use(int a)
    CERASTES_REQUIRES(0 <= a && a <= 100)
    CERASTES_ENSURES(r, r == a + 1)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  return a + 1;
}
