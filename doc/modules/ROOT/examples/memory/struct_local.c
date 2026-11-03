// expect: verified
// expect: native-ok
// mode: c23, c++20
// A struct local: an initializer, a member update, and a member passed by address.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

struct point {
  int x;
  int y;
};

void inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < INT_MAX)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p += 1;
}

int moved(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct point pt = {.x = 1, .y = 0};
  pt.y = 5;
  inc(&pt.x);
  return pt.x + pt.y;
}
// end::source[]
