// expect: golden
// expect: native-ok
// Pins: must-match for templates: a function template declared with a contract
// and defined with the same contract under a renamed parameter; the prototype
// and the instance each carry it once per declaration, and the instance too.
#include <cerastes.h>

template <class T> T dbl(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v);

template <class T> T dbl(T w) CERASTES_REQUIRES(w >= 0) CERASTES_ENSURES(r, r >= w)
{
  return w + w;
}

long use(long x)
{
  return dbl(x);
}
