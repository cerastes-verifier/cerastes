// expect: frontend-error cerastes-annotation_on_lambda @ L14
// expect: native-ok
// twin: tests/annotations/contracts/literal_spelling.cpp
// Mutation of the twin: a contract between the lambda-introducer and the parameters (C++23 spelling, accepted by EDG in C++20): it attaches to operator(); EDG's own placement check may report it first (pin on the first run).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

#ifdef __CERASTES__
int f(int x) [[cerastes::requires(x > 0)]] [[cerastes::ensures(r, r > x)]];
[[cerastes::predicate]] inline bool pos(int x) { return x > 0; }
int g(int n)
{
  int i = 0;
  [[maybe_unused]] auto l = [] [[cerastes::requires(true)]] () {};
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
