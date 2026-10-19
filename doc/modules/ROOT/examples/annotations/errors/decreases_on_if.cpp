// expect: frontend-error attr_requires_iteration_statement @ L9
// Before a statement, decreases belongs to a loop.
// tag::bad[]
#include <cerastes.h>

int pick(int a, int b) CERASTES_REQUIRES(a >= 0 && b >= 0)
{
  int m = a;
  CERASTES_DECREASES(a) if (a < b) { // <- error
    m = b;
  }
  return m;
}
// end::bad[]
