// expect: frontend-error attr_redeclaration_mismatch @ L8
// An explicit specialization with a different contract from the primary template's.
// tag::bad[]
#include <cerastes.h>

template <class T> T twice(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v);

template <> int twice<int>(int v) CERASTES_REQUIRES(v > 0) CERASTES_ENSURES(r, r >= v) // <- error
{
  return v + v;
}
// end::bad[]
