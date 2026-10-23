// expect: error uninitialized @ L12
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

int positive_or_none(int a)
{
  int r;
  if (a > 0) {
    r = a;
  }
  return r;
}
// end::source[]
