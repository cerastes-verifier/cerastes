// expect: frontend-error cerastes-spec_call_in_code @ L13
// The program must not call a predicate: it exists for annotations.
// tag::bad[]
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

int caller(int x)
{
  return nonneg(x) ? 1 : 0; // <- error
}
// end::bad[]
