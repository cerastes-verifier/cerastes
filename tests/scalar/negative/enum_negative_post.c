// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/scalar/enum_negative.c
// Pins: 0018 item 3: -2 is a value of enum sign, so the magnitude can be 2, not at most 1
#include <cerastes.h>

enum sign { NEG = -1, ZERO, POS };

enum sign classify(int x)
    CERASTES_REQUIRES(-2 <= x && x <= 1)
    CERASTES_ENSURES(r, (int)r == x)
{
  enum sign s = (enum sign)x;
  return s;
}

int magnitude(enum sign s)
    CERASTES_ENSURES(r, 0 <= r && r <= 1 && (r == (int)s || r == -(int)s))
{
  int v = (int)s;
  if (v < 0) {
    return -v;
  }
  return v;
}

int use(void)
    CERASTES_ENSURES(r, r == 4)
{
  enum sign low = classify(-2);
  enum sign mid = classify(0);
  enum sign high = classify(1);
  int a = magnitude(low);
  int b = magnitude(NEG);
  int c = magnitude(mid);
  int d = magnitude(high);
  return a + b + c + d;
}
