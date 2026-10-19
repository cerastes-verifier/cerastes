// expect: frontend-error cerastes-old_outside_ensures @ L8
// A predicate or pure function has no entry state: old belongs in an ensures.
// tag::bad[]
#include <cerastes.h>

CERASTES_PURE int previous(int x)
{
  return cerastes::old(x); // <- error
}
// end::bad[]
