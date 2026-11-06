// expect: verified
// expect: native-ok
// mode: c23, c++20
// A function returns the address of its static local and promises that the result points to a live object; its caller may use the pointer.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int *where(void)
    CERASTES_ENSURES(r, cerastes_live(r))
{
  static int s = 5;
  return &s;
}

int use_where(void)
{
  int *p = where();
  if (p == NULL) {
    return 1;
  }
  return 0;
}
// end::source[]
