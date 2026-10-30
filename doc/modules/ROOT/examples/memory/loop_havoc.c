// expect: error postcondition @ L22
// mode: c23, c++20
// A write through an int pointer in a loop may change every int local whose address is taken: keep is no longer known to be 7.
// tag::source[]
#include <cerastes.h>

int havoc(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 7)
{
  int keep = 7;
  int last = 0;
  int *pk = &keep;
  int *pl = &last;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *pl = i;
    i += 1;
  }
  return *pk;
}
// end::source[]
