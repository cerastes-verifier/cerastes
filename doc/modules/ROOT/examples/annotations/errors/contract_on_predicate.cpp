// expect: frontend-error cerastes-contract_on_predicate @ L6
// A predicate has no contract.
// tag::bad[]
#include <cerastes.h>

CERASTES_PREDICATE bool positive(int x) CERASTES_REQUIRES(x > -100) // <- error
{
  return x > 0;
}
// end::bad[]
