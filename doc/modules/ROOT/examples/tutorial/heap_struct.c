// expect: verified
// expect: native-ok
// mode: c23, c++20
// A heap structure returned to the caller, who frees it.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct point {
  int x;
  int y;
};

struct point *make_point(int x, int y)
    CERASTES_ENSURES(r, r == NULL || (cerastes_owns_block(r) && r->x == x && r->y == y))
{
  struct point *p = (struct point *)malloc(sizeof(struct point));
  if (p == NULL) {
    return NULL;
  }
  p->x = x;
  p->y = y;
  return p;
}

int main(void)
{
  struct point *p = make_point(3, 4);
  if (p == NULL) {
    return 1;
  }
  int sum = p->x + p->y;
  free(p);
  return sum == 7 ? 0 : 1;
}
// end::source[]
