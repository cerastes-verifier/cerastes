// expect: error permission @ L18
// mode: c23, c++20
// A covered live(r) lets the caller use the pointer, but not read the object: live never gives access.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int *where(void)
    CERASTES_ENSURES(r, cerastes_live(r))
{
  static int s = 5;
  return &s;
}

int main(void)
{
  int *p = where();
  int v = *p;
  return v;
}
// end::source[]
