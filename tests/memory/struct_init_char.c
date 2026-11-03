// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 6, audit S7: element values that fit a character member are stored as they are
#include <cerastes.h>

struct B {
  unsigned char c;
  int y;
};

struct SB {
  signed char c;
  int y;
};

int unsigned_fits(void)
    CERASTES_ENSURES(r, r == 200)
{
  struct B b = {200, 1};
  return b.c;
}

int signed_fits(void)
    CERASTES_ENSURES(r, r == 100)
{
  struct SB b = {100, 1};
  return b.c;
}
