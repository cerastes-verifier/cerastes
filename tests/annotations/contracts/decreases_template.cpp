// expect: golden
// expect: native-ok
// Pins: decision 0013 item 1: decreases in a function template: type-checked in the template, substituted in each instance
#include <cerastes.h>


template <class T>
T down(T n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
{
  return n == 0 ? n : down(n - 1);
}

int use(int n) CERASTES_REQUIRES(n >= 0) { return down(n); }
