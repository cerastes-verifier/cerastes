// expect: frontend-error cerastes-unsupported_annotation_expression @ L9
// expect: native-ok
// twin: tests/annotations/contracts/template_fn.cpp
// Mutation of the twin: the same for a requires-expression without parameters (PENDING).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
T inc(T v) CERASTES_REQUIRES(requires { v.foo(); }) CERASTES_ENSURES(r, r > v)
{
  return v + 1;
}

int use(int x)
{
  return inc(x);
}
