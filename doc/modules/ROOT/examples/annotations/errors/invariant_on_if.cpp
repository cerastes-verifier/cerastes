// expect: frontend-error attr_requires_iteration_statement @ L8
// An invariant on a statement that is not a loop.
// tag::bad[]
#include <cerastes.h>

int f(int n)
{
  CERASTES_INVARIANT(n >= 0) if (n > 0) { // <- error
    n -= 1;
  }
  return n;
}
// end::bad[]
