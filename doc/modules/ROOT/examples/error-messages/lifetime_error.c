// expect: error lifetime @ L14
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

int compare_after_block(int a)
{
  int y = 0;
  int *p = &y;
  {
    int x = a;
    p = &x;
  }
  if (p == &y) {
    return 1;
  }
  return 0;
}
// end::source[]
