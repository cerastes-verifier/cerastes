// expect: frontend-error undefined_identifier @ L8
// A variable declared in the for statement is not in scope in its invariant.
// tag::bad[]
#include <cerastes.h>

int count(int n)
{
  CERASTES_INVARIANT(i >= 0) for (int i = 0; i < n; ++i) { // <- error
  }
  return n;
}
// end::bad[]
