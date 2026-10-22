// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 10: ordering of an enumeration with negative enumerators (value range -2 to 1), where C compares in int
#include <cerastes.h>

enum sign { NEG = -1, ZERO, POS };

int below(enum sign s)
    CERASTES_ENSURES(r, r == (s < ZERO ? 1 : 0))
{
  if (s < ZERO) {
    return 1;
  }
  return 0;
}

int cmp(enum sign a, enum sign b)
    CERASTES_ENSURES(r, r == (a < b ? -1 : (a > b ? 1 : 0)))
{
  if (a < b) {
    return -1;
  }
  if (a > b) {
    return 1;
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == -1)
{
  enum sign low = (enum sign)-2;
  return cmp(low, NEG);
}
