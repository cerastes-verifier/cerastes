// expect: frontend-error cerastes-old_outside_ensures @ L6
// old is only meaningful in ensures.
// tag::bad[]
#include <cerastes.h>

void incr(int& x) CERASTES_REQUIRES(cerastes::old(x) < 1000) // <- error
{
  x += 1;
}
// end::bad[]
