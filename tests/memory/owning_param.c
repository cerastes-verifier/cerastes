// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 3: [[cerastes::owning]] on a pointer parameter, leading and trailing position, adds requires and ensures p == NULL || owns(*p); the body tests p; callers pass &x and NULL
#include <cerastes.h>
#include <stddef.h>

void set_one(CERASTES_OWNING int *p)
{
  if (p) {
    *p = 1;
  }
}

void set_two(int *p CERASTES_OWNING)
{
  if (p != NULL) {
    *p = 2;
  }
}

int use_owning(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 0;
  set_one(&x);
  set_two(&x);
  set_one(NULL);
  x = 3;
  return x;
}
