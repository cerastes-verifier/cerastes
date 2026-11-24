// expect: error out-of-bounds @ L13
// mode: c23, c++20
// The precondition lets i be 3, one past the end of the array.
// Derived from arrays_local.c.
// tag::source[]
#include <cerastes.h>

int mark_and_read(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
    CERASTES_ENSURES(r, r == 7)
{
  int a[3] = {0, 0, 0};
  a[i] = 7;
  return a[i];
}
// end::source[]
