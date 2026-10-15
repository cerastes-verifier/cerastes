// expect: frontend-error cerastes-old_outside_ensures @ L11
// expect: native-ok
// twin: tests/annotations/contracts/never_instantiated.cpp
// Mutation of the twin: an unqualified old(v) after `using namespace cerastes;`.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

using namespace cerastes;

template <class T>
T twice(T v) CERASTES_REQUIRES(old(v) > 0) CERASTES_ENSURES(r, r > v)
{
  return v + v;
}
