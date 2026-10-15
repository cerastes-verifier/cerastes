// expect: frontend-error cerastes-generic_lambda_in_annotation @ L7
// A lambda in an annotation has concrete parameter types.
// tag::bad[]
#include <cerastes.h>

int first_nonneg(const int* a, [[maybe_unused]] int n)
  CERASTES_REQUIRES(n > 0 && cerastes::forall([](auto i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n)) // <- error
{
  return a[0];
}
// end::bad[]
