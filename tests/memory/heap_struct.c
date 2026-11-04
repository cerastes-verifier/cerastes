// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 items 2, 4: a structure allocation is one object read through the type it is converted to: its member blocks, lengths and flags come from 0043's struct_lens, and free consumes all of them
// Pre-registered from record 0047 items 2, 4, 6: verifies
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int x;
  int y;
};

int use_struct(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  struct S *s = (struct S *)malloc(sizeof(struct S));
  if (s == NULL) {
    return -1;
  }
  s->x = 1;
  s->y = 2;
  int sum = s->x + s->y;
  free(s);
  return sum;
}
