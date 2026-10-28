// expect: error null-dereference @ L14
// mode: c23, c++20
// tag::source[]
#include <stddef.h>
#include <cerastes.h>

int first_or_zero(int a)
{
  int x = a;
  int *p = NULL;
  if (a > 0) {
    p = &x;
  }
  return *p;
}
// end::source[]
