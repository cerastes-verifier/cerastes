// expect: golden
// expect: native-ok
// Pins: the spelled-out [[cerastes::...]] form, used by the macros, is
// accepted in every v0 placement without <cerastes.h> macros; under an
// ordinary compiler the #else branch is the same code with no attributes.
#include <cerastes.h>

#ifdef __CERASTES__
int f(int x) [[cerastes::requires(x > 0)]] [[cerastes::ensures(r, r > x)]];
[[cerastes::predicate]] inline bool pos(int x) { return x > 0; }
int g(int n)
{
  int i = 0;
  [[cerastes::invariant(i >= 0 && i <= n)]] while (i < n) { i += 1; }
  [[cerastes::check(i >= 0)]];
  return i;
}
#else
int f(int x);
inline bool pos(int x) { return x > 0; }
int g(int n)
{
  int i = 0;
  while (i < n) { i += 1; }
  return i;
}
#endif
