// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer that may be NULL is tested before it is dereferenced.
// tag::source[]
#include <stddef.h>
#include <cerastes.h>

int store_if_set(int set)
    CERASTES_ENSURES(r, r == (set != 0 ? 5 : 0))
{
  int x = 0;
  int *p = NULL;
  if (set != 0) {
    p = &x;
  }
  if (p != NULL) {
    *p = 5;
  }
  return x;
}
// end::source[]
