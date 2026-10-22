// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 item 6(ii): a switch over an unfixed enumeration is controlled by the promoted value (C23 6.8.4.2), so case 4 and case -1 are never taken (no error) in both modes
// Both ordinary compilers reject a case value outside the enumeration under -Wswitch -Werror, so only cerastes sees these two cases
// (it defines __CERASTES__); the code the compilers see is the same switch without them, which means the same.
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
    CERASTES_ENSURES(r, (c == BLUE && r == 3) || (c != BLUE && r == 0))
{
  switch (c) {
#ifdef __CERASTES__
  case -1:
    return 7;
#endif
  case BLUE:
    return 3;
  default:
    return 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 4)
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
