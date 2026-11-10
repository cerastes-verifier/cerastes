// expect: error leak @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_alias_call.c
// Pins: 0056 item 4 (limit 2): the same function without free(s.data) and s.data = NULL after the loop: the struct local goes out of scope owning the block the last call left in its field
// Pre-registered from record 0056 item 4 (not yet run): leak at the return, where s goes out of scope owning the block; question 160 applies to the line
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void refill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = 1;
  }
}

int f(void)
{
  struct buf s = { NULL, 0 };
  struct buf *ps = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    refill(ps);
    i = i + 1;
  }
  return 0;
}
