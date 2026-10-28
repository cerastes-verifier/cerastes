// expect: error permission @ L13
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

int read_after_block(int a)
{
  int *p;
  {
    int x = a;
    p = &x;
  }
  return *p;
}
// end::source[]
