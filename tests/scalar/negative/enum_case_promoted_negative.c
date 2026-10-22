// expect: error postcondition @ L35
// mode: c23, c++20
// twin: tests/scalar/enum_case_promoted.c
// Pins: 0018 item 6(ii): case -1 is never taken, so c == 0 does not reach the branch that returns 7
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(enum color c)
    CERASTES_ENSURES(r, (c == RED && r == 1) || (c != RED && r == 0))
{
  switch (c) {
  case RED:
    return 1;
#ifdef __CERASTES__
  case 4:
    return 2;
#endif
  default:
    return 0;
  }
}

int negative_case(enum color c)
    CERASTES_ENSURES(r, (c == BLUE && r == 3) || (c == 0 && r == 7) || (c != BLUE && c != 0 && r == 0))
{
  switch (c) {
#ifdef __CERASTES__
  case -1:
    return 7;
#endif
  case BLUE:
    return 3;
  default:
    return 0;  // c == 0 lands here
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 11)
{
  enum color odd = (enum color)3;
  int a = classify(RED);
  int b = classify(GREEN);
  int c = classify(BLUE);
  int d = classify(odd);
  int e = negative_case(RED);
  int f = negative_case(GREEN);
  int g = negative_case(BLUE);
  int h = negative_case(odd);
  return a + b + c + d + e + f + g + h;
}
