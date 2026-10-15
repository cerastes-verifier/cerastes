// expect: frontend-error cerastes-fold_callee_not_predicate @ L13
// fold needs a call to a predicate function.
// tag::bad[]
#include <cerastes.h>

CERASTES_PURE int twice(int x)
{
  return x + x;
}

int f(int x)
{
  CERASTES_FOLD(twice(x)); // <- error
  return x;
}
// end::bad[]
