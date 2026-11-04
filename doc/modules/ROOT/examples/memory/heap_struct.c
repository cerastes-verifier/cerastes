// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct point {
  int x;
  int y;
};

int main(void)
{
  struct point *p = (struct point *)calloc(1, sizeof(struct point));
  if (p == NULL) {
    return 1;
  }
  CERASTES_CHECK(p->x == 0 && p->y == 0);
  p->x = 3;
  p->y = 4;
  int sum = p->x + p->y;
  free(p);
  return sum == 7 ? 0 : 1;
}
// end::source[]
