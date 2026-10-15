// expect: golden
// expect: native-ok
// Pins: a predicate template and a pure function template used in the contract of a
// function template: the calls are dependent in the prototype (callee and arguments)
// and substituted in each instance (int and long).
#include <cerastes.h>

template <class T>
CERASTES_PREDICATE bool nonneg(T v)
{
  return v >= 0;
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
