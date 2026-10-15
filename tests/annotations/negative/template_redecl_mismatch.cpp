// expect: frontend-error attr_redeclaration_mismatch @ L10
// expect: native-ok
// twin: tests/annotations/contracts/template_redecl.cpp
// Mutation of the twin: a function template's definition whose ensures differs from its declaration's.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T> T dbl(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v);

template <class T> T dbl(T w) CERASTES_REQUIRES(w >= 0) CERASTES_ENSURES(r, r > w)
{
  return w + w;
}

long use(long x)
{
  return dbl(x);
}
