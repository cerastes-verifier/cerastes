// expect: error lifetime @ L15
// mode: c23, c++20
// A pointer to a local of a block that has ended is not even tested.
// tag::source[]
#include <cerastes.h>

int use_after_block(int a)
{
  int found = 0;
  int *p;
  {
    int x = a;
    p = &x;
  }
  if (p) {
    found = 1;
  }
  return found;
}
// end::source[]
