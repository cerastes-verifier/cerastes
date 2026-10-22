// expect: verified
// expect: native-ok
// mode: c23, c++20
// An enumeration has the values of its smallest bit-field (0 to 3 here), a checked conversion into it, case labels and ==.
// tag::source[]
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color from_int(int x)
    CERASTES_REQUIRES(0 <= x && x <= 3)
    CERASTES_ENSURES(r, (int)r == x)
{
  return (enum color)x;
}

int weight(enum color c)
    CERASTES_ENSURES(r, r == (c == RED ? 10 : c == GREEN ? 20 : c == BLUE ? 30 : 0))
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

int is_red(enum color c)
    CERASTES_ENSURES(r, r == (c == RED))
{
  if (c == RED) {
    return 1;
  }
  return 0;
}
// end::source[]

int edges(void)
    CERASTES_ENSURES(r, r == 3)
{
  enum color lo = from_int(0);
  enum color hi = from_int(3);
  return (int)lo + (int)hi;
}
