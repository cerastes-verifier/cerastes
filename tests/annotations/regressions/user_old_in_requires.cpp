// expect: golden
// expect: native-ok
// twin-exempt: regression test for a false positive: mine::old in a template's requires is the user's function
// Pins: a qualified mine::old(x) in a template's precondition is not cerastes::old: no old_outside_ensures.
#include <cerastes.h>

namespace mine {
template <class T>
T old(T v)
{
  return v;
}
}

template <class T>
int f(T x) CERASTES_REQUIRES(mine::old(x) > 0)
{
  return static_cast<int>(x);
}

int use()
{
  return f(1);
}
