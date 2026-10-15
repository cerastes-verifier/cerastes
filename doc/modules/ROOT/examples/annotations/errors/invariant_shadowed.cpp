// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L10
// The invariant names i, but the loop declares its own i: the invariant would be about the outer one.
// tag::bad[]
#include <cerastes.h>

int count(int n) CERASTES_REQUIRES(n >= 0)
{
  [[maybe_unused]] int i;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == i) for (int i = 0; i < n; ++i) { // <- error
    c += 1;
  }
  return c;
}
// end::bad[]
