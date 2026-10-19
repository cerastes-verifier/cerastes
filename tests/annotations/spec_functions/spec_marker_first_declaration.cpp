// expect: golden
// expect: native-ok
// Pins: 0006 item 10: the marker of a predicate or pure function on a function of the user's own (declaration and definition); no special member function is marked
#include <cerastes.h>


CERASTES_PURE int twice(int x);
CERASTES_PREDICATE bool positive(int x);

CERASTES_PURE int twice(int x)
{
  return x + x;
}

CERASTES_PREDICATE bool positive(int x)
{
  return x > 0;
}

struct S {
  int a;
  S& operator=(const S&) = default;
};
