// expect: frontend-error cerastes-generic_lambda_in_annotation @ L9
// expect: native-ok
// twin: tests/annotations/contracts/never_instantiated.cpp
// Mutation of the twin: the same lambda in a template that is never instantiated: EDG never reaches the instantiation error, so cerastes's own check is the one that fires.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
T twice(T v) CERASTES_REQUIRES([](auto a, auto... b) -> bool { return a > 0; }(v)) CERASTES_ENSURES(r, r > v)
{
  return v + v;
}
