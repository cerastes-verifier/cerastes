// expect: frontend-error attr_redeclaration_mismatch @ L10
// A contract with a lambda cannot be repeated: lambdas compare by identity.
// tag::bad[]
#include <cerastes.h>

bool all_nonneg(const int* a, int n)
  CERASTES_REQUIRES(n >= 0 && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n));

bool all_nonneg(const int* a, int n)
  CERASTES_REQUIRES(n >= 0 && cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n)) // <- error
{
  for (int i = 0; i < n; ++i) {
    if (a[i] < 0) {
      return false;
    }
  }
  return true;
}
// end::bad[]
