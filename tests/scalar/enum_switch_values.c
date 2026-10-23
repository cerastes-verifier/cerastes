// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 11: a case is judged by value (case 1 is GREEN, (enum color)2 is BLUE), and duplicate enumerator values are covered together
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
enum dup { A = 0, B = 0, C = 2 };

int by_value(enum color c)
    CERASTES_ENSURES(r, r == (c == RED ? 1 : (c == GREEN ? 2 : (c == BLUE ? 3 : 0))))
{
  switch (c) {
  case RED:
    return 1;
  case 1:
    return 2;
  case (enum color)2:
    return 3;
  }
  return 0;
}

int dup_cover(enum dup d)
    CERASTES_ENSURES(r, r == (d == A ? 1 : (d == C ? 2 : 0)))
{
  switch (d) {
  case A:
    return 1;
  case C:
    return 2;
  }
  return 0;
}

int with_default(enum color c)
    CERASTES_ENSURES(r, r == (c == RED ? 1 : 0))
{
  switch (c) {
  case RED:
    return 1;
  default:
    return 0;
  }
}

int by_int(enum color c)
    CERASTES_ENSURES(r, r == (c == RED ? 1 : 0))
{
  switch ((int)c) {
  case 0:
    return 1;
  case 4:
    return 2;
  case -1:
    return 3;
  default:
    return 0;
  }
}
