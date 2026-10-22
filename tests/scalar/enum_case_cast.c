// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 items 6(ii) and 9: a written cast in a case label is encoded as written; (enum color)2 is BLUE in both modes
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int pick(enum color c)
    CERASTES_ENSURES(r, (c == RED && r == 1) || (c == BLUE && r == 2) || (c != RED && c != BLUE && r == 0))
{
  switch (c) {
  case RED:
    return 1;
  case (enum color)2:
    return 2;
  default:
    return 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  int a = pick(RED);
  int b = pick(BLUE);
  int c = pick(GREEN);
  return a + b + c;
}
