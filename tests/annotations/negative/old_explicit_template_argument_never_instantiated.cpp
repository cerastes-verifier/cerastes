// expect: frontend-error cerastes-old_outside_ensures @ L9
// expect: native-ok
// twin: tests/annotations/contracts/never_instantiated.cpp
// Mutation of the twin: old<T>(v) with an explicit template argument in a precondition of a template that is never instantiated.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
T twice(T v) CERASTES_REQUIRES(cerastes::old<T>(v) > 0) CERASTES_ENSURES(r, r > v)
{
  return v + v;
}
