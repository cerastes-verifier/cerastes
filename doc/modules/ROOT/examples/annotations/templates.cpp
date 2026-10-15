// expect: golden
// expect: native-ok
// A contract in a function template, checked in the template and in each instance.
#include <cerastes.h>

// tag::template[]
template <class T>
T inc(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r > v)
{
  return v + 1;
}

int use(int x)
{
  return inc(x);
}
// end::template[]
