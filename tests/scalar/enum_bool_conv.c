// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 item 6: an enumeration value is a condition and converts to bool as c != 0, in both modes
// bool is a keyword in C23 and in C++.
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int nonzero(enum color c)
    CERASTES_ENSURES(r, r == (c != RED))
{
  if (c) {
    return 1;
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
