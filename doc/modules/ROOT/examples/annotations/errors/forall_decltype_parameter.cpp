// expect: frontend-error cerastes-annotation_substitution_error @ L9
// expect: frontend-error cerastes-unsupported_annotation_expression @ L9
// A lambda that depends on T only through decltype of a function parameter is not accepted.
// tag::bad[]
#include <cerastes.h>

template <class T>
bool all_pos([[maybe_unused]] T* data, [[maybe_unused]] int n)
  CERASTES_REQUIRES(cerastes::forall([](int i, decltype(data) a, int m) { return !(0 <= i && i < m) || a[i] > 0; }, data, n)) // <- error
{
  return true;
}

bool use(int* p)
{
  return all_pos(p, 1);
}
// end::bad[]
