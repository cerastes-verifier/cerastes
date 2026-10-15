// expect: frontend-error param_not_allowed @ L26
// expect: native-ok
// twin: tests/annotations/spec_functions/forall_lambda.cpp
// Mutation of the twin: a function parameter named inside a lambda in a contract (probe F2: use the free-variable form).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int first_nonneg(const int* a, [[maybe_unused]] int n)
  CERASTES_REQUIRES(n > 0 && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n))
  CERASTES_ENSURES(r, r >= 0)
{
  return a[0];
}

int zero_all(int* a, int n)
  CERASTES_ENSURES(r, r == n && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] == 0; }, a, n))
{
  int i = 0;
  while (i < n) {
    a[i] = 0;
    i += 1;
  }
  return n;
}

bool small(int x) CERASTES_ENSURES(r, cerastes::forall([](int i) { return i >= x; }) && r == (x < 10))
{
  return x < 10;
}
