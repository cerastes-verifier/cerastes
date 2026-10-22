// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 items 3, 6 and 9: case RED is the enumerator's value; 3 is a value of enum color that no enumerator names
// The switch covers every enumerator, and the code after it handles the one value left (3).
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int weight(enum color c)
    CERASTES_ENSURES(r, (c == RED && r == 10) || (c == GREEN && r == 20) || (c == BLUE && r == 30) || (c != RED && c != GREEN && c != BLUE && r == 0))
{
  switch (c) {
  case RED:
    return 10;
  case GREEN:
    return 20;
  case BLUE:
    return 30;
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 60)
{
  int a = weight(RED);
  int b = weight(GREEN);
  int c = weight(BLUE);
  int d = weight((enum color)3);
  return a + b + c + d;
}
