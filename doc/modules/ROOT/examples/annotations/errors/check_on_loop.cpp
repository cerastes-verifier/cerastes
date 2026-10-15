// expect: frontend-error attr_requires_null_statement @ L8
// check, fold and unfold belong on a null statement, not on a loop.
// tag::bad[]
#include <cerastes.h>

int f(int n)
{
  CERASTES_CHECK(n >= 0) while (n > 0) { // <- error
    n -= 1;
  }
  return n;
}
// end::bad[]
