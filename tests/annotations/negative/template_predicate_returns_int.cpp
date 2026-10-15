// expect: frontend-error cerastes-predicate_not_bool @ L9
// expect: native-ok
// twin: tests/annotations/spec_functions/template_predicate_pure.cpp
// Mutation of the twin: a predicate template that returns int instead of bool.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
CERASTES_PREDICATE int nonneg(T v)
{
  return v >= 0 ? 1 : 0;
}

template <class T>
CERASTES_PURE T twice(T v)
{
  return v + v;
}

template <class T>
T bump(T v) CERASTES_REQUIRES(nonneg(v)) CERASTES_ENSURES(r, r >= twice(v))
{
  return v + v + 1;
}

long use(int a, long b)
{
  return bump(a) + bump(b);
}
