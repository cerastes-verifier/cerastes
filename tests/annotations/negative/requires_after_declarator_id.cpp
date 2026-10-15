// expect: frontend-error cerastes-contract_not_after_parameters @ L10
// expect: native-ok
// twin: tests/annotations/contracts/literal_spelling.cpp
// Mutation of the twin: requires after the declarator-id: x binds to the outer declaration, so EDG accepts it (CP2a); cerastes checks syntactic_location == al_post_func.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

#ifdef __CERASTES__
extern int x;
int f [[cerastes::requires(x > 0)]] (int x);
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
