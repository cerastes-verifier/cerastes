// expect: golden
// expect: native-ok
// Pins: decision 0006 item 6, limit: a contract that contains a lambda (every
// forall) is written once, on the declaration; the definition has none and has
// the declaration's. (Twin of lambda_contract_repeated, where repeating it is
// a mismatch because lambdas compare by identity.)
#include <cerastes.h>

bool all_nonneg(const int* a, int n)
  CERASTES_REQUIRES(n >= 0 && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n));

bool all_nonneg(const int* a, int n)
{
  for (int i = 0; i < n; ++i) {
    if (a[i] < 0) {
      return false;
    }
  }
  return true;
}
