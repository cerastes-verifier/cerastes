// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 4, 5: a pointer to a member of an inner block's struct is used (read, compared) inside the block, which ends with every leaf whole
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int inner(void)
    CERASTES_ENSURES(r, r == 6)
{
  int *p;
  struct S s = {1, 2};
  {
    struct S t = {5, 6};
    p = &t.y;
    if (p == &t.y) {
      s.x = *p;
    }
  }
  return s.x;
}
