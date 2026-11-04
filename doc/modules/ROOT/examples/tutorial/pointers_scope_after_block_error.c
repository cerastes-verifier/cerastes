// expect: error permission @ L19
// mode: c23, c++20
// The pointer is read after the block that holds x has ended, so x no longer exists.
// Derived from pointers_scope.c.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

int use_in_block(int a)
    CERASTES_REQUIRES(a < INT_MAX)
    CERASTES_ENSURES(r, r == a + 1)
{
  int *p;
  {
    int x = a;
    p = &x;
    *p = *p + 1;
  }
  return *p;
}
// end::source[]
