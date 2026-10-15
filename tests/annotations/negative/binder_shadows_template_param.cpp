// expect: frontend-error decl_hides_template_parameter @ L9
// expect: native-ok
// twin: tests/annotations/contracts/template_fn.cpp
// Mutation of the twin: the result name T is also a template parameter name (verify: EDG may name the scope differently).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
T inc(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(T, T > v)
{
  return v + 1;
}

int use(int x)
{
  return inc(x);
}
