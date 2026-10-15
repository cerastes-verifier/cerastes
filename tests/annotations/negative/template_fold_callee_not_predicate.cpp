// expect: frontend-error cerastes-fold_callee_not_predicate @ L25
// expect: native-ok
// twin: tests/annotations/statements/template_fold_unfold.cpp
// Mutation of the twin: fold names a function template that is not a predicate: rejected where the contract is substituted, in each instance.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
CERASTES_PREDICATE bool pos(T v)
{
  return v > 0;
}

template <class T>
T plain(T v)
{
  return v;
}

template <class T>
T keep([[maybe_unused]] T v) CERASTES_REQUIRES(pos(v))
{
  CERASTES_UNFOLD(pos(v));
  T w = v;
  CERASTES_FOLD(plain(w));
  return w;
}

long use(int a, long b)
{
  return keep(a) + keep(b);
}
