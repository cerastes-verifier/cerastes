// expect: golden
// expect: native-ok
// Pins: a template that is never instantiated still shows its contract on the
// prototype routine (G1 check 2 for registered attributes); there is no
// instance routine.
#include <cerastes.h>

template <class T>
T twice(T v) CERASTES_REQUIRES(v > 0) CERASTES_ENSURES(r, r > v)
{
  return v + v;
}
