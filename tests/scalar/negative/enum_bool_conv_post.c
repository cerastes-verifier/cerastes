// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_bool_conv.c
// Pins: 0018 item 6: c is true for GREEN and BLUE, but the branch returns 2, which the ensures does not allow
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int nonzero(enum color c)
    CERASTES_ENSURES(r, r == (c != RED))
{
  if (c) {
    return 2;
  }
  return 0;
}

int as_flag(enum color c)
    CERASTES_ENSURES(r, r == (c != RED))
{
  bool b = c;
  if (b) {
    return 1;
  }
  return 0;
}

int explicit_flag(enum color c)
    CERASTES_ENSURES(r, r == (c != RED))
{
  bool b = (bool)c;
  if (b) {
    return 1;
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 4)
{
  int a = nonzero(GREEN);
  int b = as_flag(BLUE);
  int c = explicit_flag(GREEN);
  int d = nonzero(RED);
  int e = as_flag(RED);
  int f = explicit_flag(BLUE);
  return a + b + c + d + e + f;
}
