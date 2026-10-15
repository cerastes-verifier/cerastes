// expect: golden
// expect: native-ok
// twin-exempt: regression test for a false positive: mine::old in a template's check is the user's function
// Pins: a qualified mine::old(x) in a check statement of a template is not cerastes::old: no old_outside_ensures.
#include <cerastes.h>

namespace mine {
template <class T>
bool old(T v)
{
  return v > 0;
}
}

template <class T>
void g(T x)
{
  CERASTES_CHECK(mine::old(x));
  (void)x;
}

int use()
{
  g(1);
  return 0;
}
