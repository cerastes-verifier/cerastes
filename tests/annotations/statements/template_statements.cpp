// expect: golden
// expect: native-ok
// Pins: invariant and check inside a function template body: dependent in the
// prototype, concrete in the instance, each instance showing its statements.
#include <cerastes.h>

template <class T>
T sum_n(T n) CERASTES_REQUIRES(n >= 0)
{
  T i = 0;
  T s = 0;
  CERASTES_INVARIANT(i >= 0 && i <= n && s >= 0) while (i < n) {
    s += i;
    i += 1;
  }
  CERASTES_CHECK(s >= 0);
  return s;
}

int use(int n)
{
  return sum_n(n);
}
