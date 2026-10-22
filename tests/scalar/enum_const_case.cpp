// expect: verified
// expect: native-ok
// Pins: 0018 items 6(ii) and 9 with 0017: in C++ a const enumeration object is a constant case value (C refuses it, error #1901)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

const color DEF = GREEN;

int pick(color c)
    CERASTES_ENSURES(r, (c == GREEN && r == 1) || (c != GREEN && r == 0))
{
  switch (c) {
  case DEF:
    return 1;
  default:
    return 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a = pick(GREEN);
  int b = pick(BLUE);
  return a + b;
}
