// expect: golden
// expect: native-ok
// A contract with a lambda is written once, on one declaration.
#include <cerastes.h>

// tag::once[]
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
// end::once[]
