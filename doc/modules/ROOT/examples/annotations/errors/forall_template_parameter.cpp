// expect: frontend-error undefined_identifier @ L8
// A lambda in a function template's contract cannot name a template parameter.
// tag::bad[]
#include <cerastes.h>

template <class T>
bool all_nonneg(const T* a, int n)
  CERASTES_REQUIRES(n >= 0 && cerastes::forall([](int i, const T* p, int m) { return !(0 <= i && i < m) || p[i] >= 0; }, a, n)) // <- error
{
  return true;
}

int use(const int* a)
{
  return all_nonneg(a, 3);
}
// end::bad[]
