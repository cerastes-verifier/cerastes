// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 item 1 and 0017: a const global of enumeration type is a named constant, used in code and contracts
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

const enum color DEF = GREEN;

int is_default(enum color c)
    CERASTES_ENSURES(r, r == (c == DEF))
{
  if (c == DEF) {
    return 1;
  }
  return 0;
}

enum color default_color(void)
    CERASTES_ENSURES(r, r == DEF)
{
  return DEF;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a = is_default(default_color());
  int b = is_default(GREEN);
  return a + b;
}

int from_const(void)
    CERASTES_ENSURES(r, r == 1)
{
  int i = DEF;
  return i;
}
