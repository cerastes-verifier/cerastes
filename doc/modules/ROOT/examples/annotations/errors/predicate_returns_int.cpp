// expect: frontend-error cerastes-predicate_not_bool @ L6
// A predicate returns bool.
// tag::bad[]
#include <cerastes.h>

CERASTES_PREDICATE int positive(int x) // <- error
{
  return x > 0;
}
// end::bad[]
