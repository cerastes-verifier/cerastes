// expect: frontend-error cerastes-old_outside_ensures @ L13
// expect: native-ok
// twin: tests/annotations/contracts/never_instantiated.cpp
// Mutation of the twin: a using-directive for cerastes in BLOCK scope, then old(v) in a check.
// Pins: CP7 regression — spec functions brought in anywhere but file scope were missed; dependent names fail closed.
#include <cerastes.h>

template <class T>
T twice(T v) CERASTES_ENSURES(r, r > v)
{
  using namespace cerastes;
  T w = v + v;
  CERASTES_CHECK(old(v) > 0);
  return w;
}
