// expect: frontend-error expr_not_class @ L10
// expect: frontend-error bad_attribute_template_substitution @ L10
// expect: native-ok
// twin: tests/annotations/contracts/template_fn.cpp
// Mutation of the twin: ill-typed only for T=int: dependent in the prototype (accepted), an error in the instance, reported at the contract's line with the instantiation context (0006 item 7).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
T inc(T v) CERASTES_REQUIRES(v.size() >= 0) CERASTES_ENSURES(r, r > v)
{
  return v + 1;
}

int use(int x)
{
  return inc(x);
}
