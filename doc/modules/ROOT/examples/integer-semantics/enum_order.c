// expect: verified
// expect: native-ok
// mode: c23, c++20
// Ordering two values of one enumeration, or a value and one of its enumerators.
// tag::source[]
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int warm(enum color c)
    CERASTES_ENSURES(r, r == (c < BLUE ? 1 : 0))
{
  if (c < BLUE) {
    return 1;
  }
  return 0;
}

int ordered(enum color a, enum color b)
    CERASTES_ENSURES(r, r == (a <= b ? 1 : 0))
{
  if (a <= b) {
    return 1;
  }
  return 0;
}
// end::source[]
