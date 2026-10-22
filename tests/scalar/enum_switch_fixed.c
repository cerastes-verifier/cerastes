// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 11: fixed enumerations (unsigned char, unsigned long) with every enumerator listed need no default
#include <cerastes.h>

enum uc : unsigned char { U0, U1 };
enum ul : unsigned long { L0, L1 };

int small(enum uc e)
    CERASTES_ENSURES(r, r == (e == U0 ? 1 : (e == U1 ? 2 : 0)))
{
  switch (e) {
  case U0:
    return 1;
  case U1:
    return 2;
  }
  return 0;
}

int big(enum ul e)
    CERASTES_ENSURES(r, r == (e == L0 ? 3 : (e == L1 ? 4 : 0)))
{
  switch (e) {
  case L0:
    return 3;
  case L1:
    return 4;
  }
  return 0;
}
