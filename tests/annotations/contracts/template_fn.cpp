// expect: golden
// expect: native-ok
// Pins: decision 0006 item 7: a contract in a function template is dependent
// in the prototype (template-param types) and concrete in the instance
// (tk_integer); the instance carries its own copy of the attribute.
#include <cerastes.h>

template <class T>
T inc(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r > v)
{
  return v + 1;
}

int use(int x)
{
  return inc(x);
}
