// expect: frontend-error unattached_attribute @ L10
// expect: native-ok
// twin: tests/annotations/contracts/literal_spelling.cpp
// Mutation of the twin: an attribute that appertains to no entity (a class definition with no declarator).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

#ifdef __CERASTES__
int f(int x) [[cerastes::requires(x > 0)]] [[cerastes::ensures(r, r > x)]];
[[cerastes::predicate]] struct pos_tag {};
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
