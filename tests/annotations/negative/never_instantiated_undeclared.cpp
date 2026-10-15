// expect: frontend-error undefined_identifier @ L9
// expect: native-ok
// twin: tests/annotations/contracts/never_instantiated.cpp
// Mutation of the twin: found in the prototype although the template is never instantiated.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
T twice(T v) CERASTES_REQUIRES(w > 0) CERASTES_ENSURES(r, r > v)
{
  return v + v;
}
