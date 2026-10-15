// expect: golden
// expect: native-ok
// Pins: fold and unfold inside a function template, naming a predicate that is a
// template itself: the argument is a call whose callee and arguments are dependent in
// the prototype and substituted in each instance (int and long).
#include <cerastes.h>

template <class T>
CERASTES_PREDICATE bool pos(T v)
{
  return v > 0;
}

template <class T>
T keep([[maybe_unused]] T v) CERASTES_REQUIRES(pos(v))
{
  CERASTES_UNFOLD(pos(v));
  T w = v;
  CERASTES_FOLD(pos(w));
  return w;
}

long use(int a, long b)
{
  return keep(a) + keep(b);
}
