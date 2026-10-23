// expect: error conversion @ L10
// mode: c23, c++20
// With i == -1, i + 0u converts -1 to unsigned: a conversion error, not UINT_MAX.
// tag::source[]
#include <cerastes.h>

int sum_is_positive(int i)
    CERASTES_REQUIRES(i == -1)
{
  return i + 0u > 0u;
}
// end::source[]
