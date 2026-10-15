// expect: frontend-error attr_redeclaration_mismatch @ L12
// expect: native-ok
// twin: tests/annotations/contracts/lambda_contract_once.cpp
// Mutation of the twin: documented limitation: a contract with a lambda compares by identity, so repeating it is a mismatch even when the text is identical; write it once.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

bool all_nonneg(const int* a, int n)
  CERASTES_REQUIRES(n >= 0 && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n));

bool all_nonneg(const int* a, int n)
  CERASTES_REQUIRES(n >= 0 && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n))
{
  for (int i = 0; i < n; ++i) {
    if (a[i] < 0) {
      return false;
    }
  }
  return true;
}
