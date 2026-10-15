// expect: frontend-error cerastes-generic_lambda_in_annotation @ L9
// expect: native-ok
// twin: tests/annotations/spec_functions/forall_lambda.cpp
// Mutation of the twin: the quantified variable of a forall lambda declared auto (the lambda must be written with the type).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int first_nonneg(const int* a, [[maybe_unused]] int n)
  CERASTES_REQUIRES(n > 0 && cerastes::forall([](auto i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n))
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

bool small(int x) CERASTES_ENSURES(r, cerastes::forall([](int i) { return i >= 0; }) && r == (x < 10))
{
  return x < 10;
}
